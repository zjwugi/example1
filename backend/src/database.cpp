#include "database.h"

#include <sstream>
#include <stdexcept>

SQLiteConnectionPool::SQLiteConnectionPool(std::string db_path, size_t size)
    : db_path_(std::move(db_path)) {
  for (size_t i = 0; i < size; ++i) {
    sqlite3* db = nullptr;
    if (sqlite3_open_v2(db_path_.c_str(), &db,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
                        nullptr) != SQLITE_OK) {
      throw std::runtime_error(std::string("Failed to open DB: ") + sqlite3_errmsg(db));
    }
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA foreign_keys=ON;", nullptr, nullptr, nullptr);
    all_.push_back(db);
    free_.push(db);
  }
}

SQLiteConnectionPool::~SQLiteConnectionPool() {
  for (auto* conn : all_) sqlite3_close(conn);
}

SQLiteConnectionPool::Lease::Lease(SQLiteConnectionPool& pool, sqlite3* conn) : pool_(pool), conn_(conn) {}
SQLiteConnectionPool::Lease::~Lease() { pool_.release(conn_); }

SQLiteConnectionPool::Lease SQLiteConnectionPool::acquire() {
  std::unique_lock lock(mu_);
  cv_.wait(lock, [&] { return !free_.empty(); });
  auto* conn = free_.front();
  free_.pop();
  return Lease(*this, conn);
}

void SQLiteConnectionPool::release(sqlite3* conn) {
  {
    std::lock_guard lock(mu_);
    free_.push(conn);
  }
  cv_.notify_one();
}

QueryResult SQLiteConnectionPool::execute_readonly_sql(const std::string& sql) {
  auto lease = acquire();
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(lease.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error(sqlite3_errmsg(lease.get()));
  }

  QueryResult result;
  const int col_count = sqlite3_column_count(stmt);
  for (int i = 0; i < col_count; ++i) result.columns.emplace_back(sqlite3_column_name(stmt, i));

  while (true) {
    const int rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) break;
    if (rc != SQLITE_ROW) {
      sqlite3_finalize(stmt);
      throw std::runtime_error(sqlite3_errmsg(lease.get()));
    }

    std::vector<std::string> row;
    for (int i = 0; i < col_count; ++i) {
      const unsigned char* txt = sqlite3_column_text(stmt, i);
      row.emplace_back(txt ? reinterpret_cast<const char*>(txt) : "NULL");
    }
    result.rows.push_back(std::move(row));
  }

  sqlite3_finalize(stmt);
  return result;
}

std::string SQLiteConnectionPool::introspect_schema() {
  std::ostringstream oss;
  auto tables = execute_readonly_sql(
      "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name;");
  for (const auto& row : tables.rows) {
    const auto& t = row[0];
    oss << "Table: " << t << "\n";
    auto cols = execute_readonly_sql("PRAGMA table_info(" + t + ");");
    for (const auto& c : cols.rows) {
      oss << "  - " << c[1] << " " << c[2] << (c[3] == "1" ? " NOT NULL" : "")
          << (c[5] == "1" ? " PRIMARY KEY" : "") << "\n";
    }
  }
  return oss.str();
}
