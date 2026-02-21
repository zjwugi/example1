#pragma once

#include <string>
#include <vector>

std::string json_escape(const std::string& s);
std::string extract_question(const std::string& request_body);
std::string make_json_array(const std::vector<std::string>& vals);
std::string make_json_2d_array(const std::vector<std::vector<std::string>>& vals);
std::string parse_openai_sql_content(const std::string& json_text);
