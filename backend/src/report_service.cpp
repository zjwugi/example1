#include "report_service.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

ReportService::ReportService(SQLiteConnectionPool& db, LLMClient& llm) : db_(db), llm_(llm) {}

bool ReportService::is_safe_sql(const std::string& sql) {
  std::string lower = sql;
  std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });

  if (!(lower.rfind("select", 0) == 0 || lower.rfind("with", 0) == 0)) return false;
  const std::string banned[] = {"insert", "update", "delete", "drop", "alter", "attach", "detach", "pragma"};
  for (const auto& k : banned) {
    if (lower.find(k) != std::string::npos) return false;
  }
  if (lower.find(';') != std::string::npos) return false;
  return true;
}

ReportPayload ReportService::run_nl_query(const std::string& question) {
  const auto schema = db_.introspect_schema();
  auto sql = llm_.generate_sql(question, schema);
  sql.erase(std::remove(sql.begin(), sql.end(), '`'), sql.end());
  if (!is_safe_sql(sql)) {
    throw std::runtime_error("Generated SQL rejected by safety guard: " + sql);
  }

  auto result = db_.execute_readonly_sql(sql);
  return {question, sql, std::move(result)};
}
