# NL2SQL BI 报表系统（C++ + SQLite3 + Vue）

这是一个完整的 **自然语言转 SQL（NL2SQL）报表系统** 示例：
- 后端：C++20 + cpp-httplib + sqlite3 + libcurl
- 数据库：SQLite3（WAL + 连接池）
- LLM：兼容 OpenAI Chat Completions API
- 前端：Vue3 + Vite

## 架构说明

1. 前端将用户自然语言问题提交到 `/api/query`。
2. 后端读取 SQLite 表结构（schema introspection）。
3. 后端调用 LLM，要求生成只读 SQL。
4. 后端执行 SQL 并返回报表 JSON（列、行、行数、SQL）。
5. 前端渲染表格报表。

## 目录

```text
backend/
  CMakeLists.txt
  include/
  src/
frontend/
  package.json
  src/
```

## 后端启动

### 依赖
- cmake >= 3.20
- C++20 编译器
- sqlite3 dev
- libcurl dev

### 构建

```bash
cd backend
cmake -S . -B build
cmake --build build -j
```

### 运行

```bash
export LLM_API_KEY=你的Key
export LLM_BASE_URL=https://api.openai.com
export LLM_MODEL=gpt-4o-mini
export DB_PATH=./data/bi.db
export DB_POOL_SIZE=8
export PORT=8080

./build/nl2sql_backend
```

## 前端启动

```bash
cd frontend
npm install
npm run dev
```

打开：`http://localhost:5173`

## API

### POST /api/query

请求：

```json
{
  "question": "统计每个区域销售额"
}
```

返回：

```json
{
  "question": "...",
  "sql": "SELECT ...",
  "columns": ["region", "total_amount"],
  "rows": [["East", "12000"]],
  "row_count": 1
}
```

## 安全策略（示例）
- 仅允许 `SELECT` / `WITH`。
- 拦截 `INSERT/UPDATE/DELETE/DROP/ALTER/ATTACH/DETACH/PRAGMA`。
- 禁止 `;` 防止多语句注入。

> 生产场景建议再增加 SQL AST 解析白名单、行级权限、查询超时与审计。
