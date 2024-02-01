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
#include "scales_checker.h"

namespace iyfc {

ScalesChecker::ScalesChecker(Dag &g, NodeMapOptional<std::uint32_t> &scales,
                             NodeMap<DataType> &types)
    : m_dag(g), m_scales(g), m_types(types) {}

void ScalesChecker::operator()(const NodePtr &node) {
  // Must only be used with forward pass traversal
  if (m_types[node] == DataType::Raw) {
    return;
  }
  auto &operands = node->getOperands();

  // Nothing to do for source terms
  if (node->m_op_type == OpType::Input || node->m_op_type == OpType::Encode) {
    m_scales[node] = node->get<EncodeAtScaleAttr>();
    if (m_scales.at(node) == 0) {
      if (node->m_op_type == OpType::Input) {
        warn("Dag has an input with 0 scale");
      } else {
        warn("Compiled dag results in a 0 scale node");
      }
    }
  } else if (node->m_op_type == OpType::Mul) {
    assert(node->numOperands() == 2);
    std::uint32_t scale = 0;
    for (auto &operand : operands) {
      scale += m_scales.at(operand);
    }
    if (scale == 0) {
      warn("Compiled dag results in a 0 scale node");
    }
    m_scales[node] = scale;
  } else if (node->m_op_type == OpType::Rescale) {
    assert(node->numOperands() == 1);
    auto divisor = node->get<RescaleDivisorAttr>();
    auto operandScale = m_scales.at(node->operandAt(0));
    std::uint32_t scale = operandScale - divisor;
    if (scale == 0) {
      warn("Compiled dag results in a 0 scale node");
    }
    m_scales[node] = scale;

  } else if (isAdditionOp(node->m_op_type)) {
    std::uint32_t scale = 0;
    for (auto &operand : operands) {
      if (scale == 0) {
        scale = m_scales.at(operand);
      } else {
        if (scale != m_scales.at(operand)) {
          warn(
              "Addition or subtraction in dag has "
              "operands of non-equal scale");
        }
      }
    }
    if (scale == 0) {
      warn("Compiled dag results in a 0 scale node");
    }
    m_scales[node] = scale;
  } else {
    auto scale = m_scales.at(node->operandAt(0));
    if (scale == 0) {
      warn("Compiled dag results in a 0 scale node");
    }
    m_scales[node] = scale;
  }
}

void ScalesChecker::free(const NodePtr &node) {
  // No-m_op_type
}

bool ScalesChecker::isAdditionOp(const OpType &op_code) {
  return ((op_code == OpType::Add) || (op_code == OpType::Sub));
}

}  // namespace iyfc
