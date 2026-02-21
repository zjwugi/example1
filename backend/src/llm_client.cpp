#include "llm_client.h"
#include "http_utils.h"

#include <curl/curl.h>
#include <stdexcept>

namespace {
size_t write_cb(void* contents, size_t size, size_t nmemb, void* userp) {
  auto* out = reinterpret_cast<std::string*>(userp);
  out->append(reinterpret_cast<char*>(contents), size * nmemb);
  return size * nmemb;
}
}  // namespace

LLMClient::LLMClient(std::string base_url, std::string model, std::string api_key)
    : base_url_(std::move(base_url)), model_(std::move(model)), api_key_(std::move(api_key)) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

std::string LLMClient::generate_sql(const std::string& user_question, const std::string& schema) {
  std::string prompt = "Schema:\n" + schema + "\nQuestion:\n" + user_question;
  std::string body =
      "{\"model\":\"" + json_escape(model_) +
      "\",\"temperature\":0.1,\"messages\":["
      "{\"role\":\"system\",\"content\":\"You are a SQLite expert. Return only one read-only SQL query without markdown. Never use INSERT/UPDATE/DELETE/ALTER/DROP/ATTACH/DETACH/PRAGMA.\"},"
      "{\"role\":\"user\",\"content\":\"" + json_escape(prompt) + "\"}]" +
      "}";

  std::string url = base_url_ + "/v1/chat/completions";
  std::string response;

  CURL* curl = curl_easy_init();
  if (!curl) throw std::runtime_error("curl init failed");

  struct curl_slist* headers = nullptr;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key_).c_str());

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

  CURLcode rc = curl_easy_perform(curl);
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  if (rc != CURLE_OK) throw std::runtime_error(curl_easy_strerror(rc));

  return parse_openai_sql_content(response);
}
