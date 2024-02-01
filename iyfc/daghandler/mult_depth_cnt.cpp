

/*
 *
 * MIT License
 * Copyright 2023 The IDEA Authors. All rights reserved.
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "mult_depth_cnt.h"

namespace iyfc {
bool isMultiplicationOp(const OpType &op_code) {
  return (op_code == OpType::Mul);
}

MultDepthCnt::MultDepthCnt(Dag &g, NodeMap<DataType> &m_type)
    : m_dag(g), m_type(m_type), m_cnt(g) {}

void MultDepthCnt::operator()(const NodePtr &node) {  // forward

  auto &operands = node->getOperands();
  if (operands.size() > 0) {
    auto &cnt = m_cnt[node];

    bool is_ciper = false;
    for (auto &operand : operands) {
      auto &op_cnt = m_cnt[operand];
      // Take the maximum of all operand counts
      if (op_cnt > cnt) {
        cnt = op_cnt;
      }
      // Check if it is a ciphertext computation - Count multiplication depth only when ciphertext operations
      if (m_type[operand] == DataType::Cipher) is_ciper = true;
    }

    // Multiplication node +1 if it is a ciphertext operation
    if (isMultiplicationOp(node->m_op_type) && is_ciper) {
      cnt = cnt + 1;
    }
  }
}

void MultDepthCnt::free(const NodePtr &node) {
  // No-op
}

// Get the maximum multiplication depth of all nodes
uint32_t MultDepthCnt::getMultDepth() {
  uint32_t max_depth = 0;
  for (const auto &entry : m_dag.getOutputs()) {
    auto &output = entry.second;
    max_depth = std::max(max_depth, m_cnt[output]);
  }
  return max_depth;
}

}  // namespace iyfc
