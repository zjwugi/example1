#pragma once

#include "database.h"
#include "llm_client.h"
#include <string>

struct ReportPayload {
  std::string question;
  std::string sql;
  QueryResult result;
};

class ReportService {
 public:
  ReportService(SQLiteConnectionPool& db, LLMClient& llm);
  ReportPayload run_nl_query(const std::string& question);

 private:
  static bool is_safe_sql(const std::string& sql);

  SQLiteConnectionPool& db_;
  LLMClient& llm_;
};
