#include "config.h"
#include "database.h"
#include "http_utils.h"
#include "llm_client.h"
#include "report_service.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sqlite3.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

static void bootstrap_demo_data(SQLiteConnectionPool& db) {
  auto lease = db.acquire();
  const char* ddl =
      "CREATE TABLE IF NOT EXISTS sales_orders ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "order_date TEXT NOT NULL,"
      "region TEXT NOT NULL,"
      "product TEXT NOT NULL,"
      "amount REAL NOT NULL,"
      "quantity INTEGER NOT NULL"
      ");";
  char* err = nullptr;
  if (sqlite3_exec(lease.get(), ddl, nullptr, nullptr, &err) != SQLITE_OK) {
    std::string e = err ? err : "DDL error";
    sqlite3_free(err);
    throw std::runtime_error(e);
  }

  sqlite3_exec(lease.get(),
               "INSERT INTO sales_orders(order_date, region, product, amount, quantity)"
               "SELECT '2026-01-01','East','Laptop',12000,4 "
               "WHERE NOT EXISTS (SELECT 1 FROM sales_orders);",
               nullptr, nullptr, nullptr);
}

static std::string make_http_json(int status, const std::string& body) {
  std::ostringstream oss;
  oss << "HTTP/1.1 " << status << (status == 200 ? " OK" : " Bad Request") << "\r\n"
      << "Content-Type: application/json\r\n"
      << "Access-Control-Allow-Origin: *\r\n"
      << "Access-Control-Allow-Methods: GET,POST,OPTIONS\r\n"
      << "Access-Control-Allow-Headers: Content-Type,Authorization\r\n"
      << "Content-Length: " << body.size() << "\r\n\r\n"
      << body;
  return oss.str();
}

static std::string make_report_json(const ReportPayload& p) {
  std::ostringstream oss;
  oss << "{\"question\":\"" << json_escape(p.question) << "\",";
  oss << "\"sql\":\"" << json_escape(p.sql) << "\",";
  oss << "\"columns\":" << make_json_array(p.result.columns) << ",";
  oss << "\"rows\":" << make_json_2d_array(p.result.rows) << ",";
  oss << "\"row_count\":" << p.result.rows.size() << "}";
  return oss.str();
}

int main() {
  try {
    auto cfg = AppConfig::from_env();
    SQLiteConnectionPool db(cfg.db_path, cfg.pool_size);
    bootstrap_demo_data(db);
    LLMClient llm(cfg.llm_base_url, cfg.llm_model, cfg.llm_api_key);
    ReportService service(db, llm);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(cfg.server_port);
    bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    listen(server_fd, 64);

    std::cout << "Server listening on :" << cfg.server_port << std::endl;
    while (true) {
      int client_fd = accept(server_fd, nullptr, nullptr);
      if (client_fd < 0) continue;
      std::string req;
      char buf[8192];
      ssize_t n = read(client_fd, buf, sizeof(buf));
      if (n > 0) req.assign(buf, n);

      std::string resp;
      try {
        if (req.rfind("OPTIONS ", 0) == 0) {
          resp = make_http_json(200, "{}");
        } else if (req.rfind("GET /api/health", 0) == 0) {
          resp = make_http_json(200, "{\"status\":\"ok\"}");
        } else if (req.rfind("POST /api/query", 0) == 0) {
          auto body_pos = req.find("\r\n\r\n");
          if (body_pos == std::string::npos) throw std::runtime_error("Malformed HTTP request");
          auto question = extract_question(req.substr(body_pos + 4));
          auto report = service.run_nl_query(question);
          resp = make_http_json(200, make_report_json(report));
        } else {
          resp = make_http_json(400, "{\"error\":\"unsupported route\"}");
        }
      } catch (const std::exception& e) {
        resp = make_http_json(400, std::string("{\"error\":\"") + json_escape(e.what()) + "\"}");
      }

      send(client_fd, resp.c_str(), resp.size(), 0);
      close(client_fd);
    }
  } catch (const std::exception& e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  }
}
