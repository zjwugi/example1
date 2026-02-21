<template>
  <div class="container">
    <h1>NL2SQL 报表系统（Vue + C++）</h1>

    <div class="card">
      <h3>输入业务问题</h3>
      <textarea v-model="question" placeholder="例如：统计每个区域近30天销售额和订单量"></textarea>
      <button :disabled="loading" @click="runQuery">{{ loading ? '分析中...' : '生成报表' }}</button>
      <p v-if="error" class="error">{{ error }}</p>
    </div>

    <div class="card" v-if="result">
      <h3>生成 SQL</h3>
      <pre>{{ result.sql }}</pre>
      <h3>结果（{{ result.row_count }} 行）</h3>
      <table>
        <thead>
          <tr>
            <th v-for="col in result.columns" :key="col">{{ col }}</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="(row, i) in result.rows" :key="i">
            <td v-for="(cell, j) in row" :key="j">{{ cell }}</td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import axios from 'axios'

const question = ref('统计每个region的销售额总和和销量总和，按销售额降序')
const result = ref(null)
const error = ref('')
const loading = ref(false)

const runQuery = async () => {
  loading.value = true
  error.value = ''
  result.value = null
  try {
    const resp = await axios.post('http://localhost:8080/api/query', { question: question.value })
    result.value = resp.data
  } catch (e) {
    error.value = e?.response?.data?.error || e.message
  } finally {
    loading.value = false
  }
}
</script>
