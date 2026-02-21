#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <sqlite3.h>
#include <string>
#include <vector>

struct QueryResult {
  std::vector<std::string> columns;
  std::vector<std::vector<std::string>> rows;
};

class SQLiteConnectionPool {
 public:
  SQLiteConnectionPool(std::string db_path, size_t size);
  ~SQLiteConnectionPool();

  SQLiteConnectionPool(const SQLiteConnectionPool&) = delete;
  SQLiteConnectionPool& operator=(const SQLiteConnectionPool&) = delete;

  class Lease {
   public:
    Lease(SQLiteConnectionPool& pool, sqlite3* conn);
    ~Lease();
    sqlite3* get() const { return conn_; }

   private:
    SQLiteConnectionPool& pool_;
    sqlite3* conn_;
  };

  Lease acquire();
  QueryResult execute_readonly_sql(const std::string& sql);
  std::string introspect_schema();

 private:
  void release(sqlite3* conn);

  std::string db_path_;
  std::mutex mu_;
  std::condition_variable cv_;
  std::queue<sqlite3*> free_;
  std::vector<sqlite3*> all_;
};
