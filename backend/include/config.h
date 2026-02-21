#pragma once

#include <cstdlib>
#include <stdexcept>
#include <string>

struct AppConfig {
  std::string db_path = "./data/bi.db";
  std::string llm_base_url = "https://api.openai.com";
  std::string llm_model = "gpt-4o-mini";
  std::string llm_api_key;
  int server_port = 8080;
  size_t pool_size = 8;

  static AppConfig from_env() {
    AppConfig c;
    if (const char* v = std::getenv("DB_PATH")) c.db_path = v;
    if (const char* v = std::getenv("LLM_BASE_URL")) c.llm_base_url = v;
    if (const char* v = std::getenv("LLM_MODEL")) c.llm_model = v;
    if (const char* v = std::getenv("LLM_API_KEY")) c.llm_api_key = v;
    if (const char* v = std::getenv("PORT")) c.server_port = std::stoi(v);
    if (const char* v = std::getenv("DB_POOL_SIZE")) c.pool_size = static_cast<size_t>(std::stoul(v));

    if (c.llm_api_key.empty()) {
      throw std::runtime_error("LLM_API_KEY is required.");
    }
    return c;
  }
};
