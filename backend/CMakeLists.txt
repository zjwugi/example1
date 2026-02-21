cmake_minimum_required(VERSION 3.20)
project(nl2sql_bs_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)

add_executable(nl2sql_backend
  src/main.cpp
  src/database.cpp
  src/llm_client.cpp
  src/report_service.cpp
  src/http_utils.cpp
)

target_include_directories(nl2sql_backend PRIVATE include)
target_link_libraries(nl2sql_backend PRIVATE SQLite::SQLite3 CURL::libcurl)
