#pragma once

#include <string>

class LLMClient {
 public:
  LLMClient(std::string base_url, std::string model, std::string api_key);
  std::string generate_sql(const std::string& user_question, const std::string& schema);

 private:
  std::string base_url_;
  std::string model_;
  std::string api_key_;
};
