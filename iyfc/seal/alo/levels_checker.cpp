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
#include "levels_checker.h"

namespace iyfc {

LevelsChecker::LevelsChecker(Dag &g, NodeMap<DataType> &types)
    : m_dag(g), m_types(types), m_levels(g) {}

void LevelsChecker::operator()(
    const NodePtr &node) {  // forward pass traversal.
  if (node->numOperands() == 0) {
    m_levels[node] = node->get<EncodeAtLevelAttr>();
  } else {
    // For nodes with multiple operands, check if the levels match
    std::size_t op_level = 0;
    for (auto &operand : node->getOperands()) {
      if (m_types[operand] == DataType::Cipher) {
        op_level = m_levels[operand];
        break;
      }
    }

    // Check if all operands have the same level
    for (auto &operand : node->getOperands()) {
      if (m_types[operand] == DataType::Cipher) {
        auto op_level2 = m_levels[operand];
        assert(op_level == op_level2);
      }
    }

    // Increment the level for rescale or modulus switch operations

    std::size_t level = op_level;
    if (isLevelAddOp(node->m_op_type) || isModSwitchOp(node->m_op_type)) {
      ++level;
    }
    m_levels[node] = level;
  }
}

void LevelsChecker::free(const NodePtr &node) {
  // No-m_op_type
}

bool LevelsChecker::isModSwitchOp(const OpType &op_code) {
  return (op_code == OpType::ModSwitch);
}

bool LevelsChecker::isLevelAddOp(const OpType &op_code) {
  return (op_code == OpType::Rescale);
}

// How to evaluate Bfv levels, to be implemented   TODO
bool BfvLevelsChecker::isLevelAddOp(const OpType &op_code) {
  return (op_code == OpType::Mul);
}


}  // namespace iyfc
