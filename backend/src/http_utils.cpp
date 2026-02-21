#include "http_utils.h"

#include <sstream>
#include <stdexcept>

std::string json_escape(const std::string& s) {
  std::ostringstream oss;
  for (char c : s) {
    switch (c) {
      case '"': oss << "\\\""; break;
      case '\\': oss << "\\\\"; break;
      case '\n': oss << "\\n"; break;
      case '\r': oss << "\\r"; break;
      case '\t': oss << "\\t"; break;
      default: oss << c;
    }
  }
  return oss.str();
}

std::string extract_question(const std::string& body) {
  const std::string key = "\"question\"";
  auto p = body.find(key);
  if (p == std::string::npos) throw std::runtime_error("Missing question field");
  p = body.find(':', p);
  p = body.find('"', p);
  if (p == std::string::npos) throw std::runtime_error("Invalid question JSON");
  ++p;
  std::string out;
  bool esc = false;
  for (; p < body.size(); ++p) {
    char c = body[p];
    if (esc) {
      if (c == 'n') out.push_back('\n');
      else out.push_back(c);
      esc = false;
      continue;
    }
    if (c == '\\') {
      esc = true;
      continue;
    }
    if (c == '"') break;
    out.push_back(c);
  }
  return out;
}

std::string make_json_array(const std::vector<std::string>& vals) {
  std::ostringstream oss;
  oss << '[';
  for (size_t i = 0; i < vals.size(); ++i) {
    if (i) oss << ',';
    oss << '"' << json_escape(vals[i]) << '"';
  }
  oss << ']';
  return oss.str();
}

std::string make_json_2d_array(const std::vector<std::vector<std::string>>& vals) {
  std::ostringstream oss;
  oss << '[';
  for (size_t i = 0; i < vals.size(); ++i) {
    if (i) oss << ',';
    oss << make_json_array(vals[i]);
  }
  oss << ']';
  return oss.str();
}

std::string parse_openai_sql_content(const std::string& json_text) {
  const std::string key = "\"content\"";
  auto p = json_text.find(key);
  if (p == std::string::npos) throw std::runtime_error("LLM response missing content");
  p = json_text.find(':', p);
  p = json_text.find('"', p);
  if (p == std::string::npos) throw std::runtime_error("LLM response invalid content");
  ++p;
  std::string out;
  bool esc = false;
  for (; p < json_text.size(); ++p) {
    char c = json_text[p];
    if (esc) {
      if (c == 'n') out.push_back('\n');
      else out.push_back(c);
      esc = false;
      continue;
    }
    if (c == '\\') {
      esc = true;
      continue;
    }
    if (c == '"') break;
    out.push_back(c);
  }
  return out;
}
