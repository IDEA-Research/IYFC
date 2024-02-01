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
#include "reduction_handler.h"

#include <map>
#include <numeric>

namespace iyfc {

bool Reduction::isReductionOp(const OpType &op_code) {
  // Addition and multiplication nodes
  return ((op_code == OpType::Add) || (op_code == OpType::Mul));
}

Reduction::Reduction(Dag &g) : m_dag(g) {}

void Reduction::operator()(NodePtr &node) {  // forward pass
  // Non-intermediate node or addition/multiplication node
  if (!node->isInternal() || !isReductionOp(node->m_op_type)) return;

  auto uses = node->getUses();
  if (uses.size() == 1) {
    auto &use = uses[0];
    if (use->m_op_type == node->m_op_type) {
      while (use->eraseOperand(node)) {
        for (auto &operand : node->getOperands()) {
          use->addOperand(operand);
        }
      }
      auto new_node_use = node->getUses();
      if (new_node_use.size() == 0) {
        // Clear nodes with no next use
        node->eraseAllOperand();
      }
    }
  }
}

bool ReductionLogExpander::isReductionOp(const OpType &op_code) {
  return ((op_code == OpType::Add) || (op_code == OpType::Mul));
}

ReductionLogExpander::ReductionLogExpander(Dag &g, NodeMap<DataType> &type)
    : m_dag(g), m_dtype(type), m_scale(g) {}

void ReductionLogExpander::operator()(NodePtr &node) {
  if (node->m_op_type == OpType::Rescale ||
      node->m_op_type == OpType::ModSwitch) {
    warn(
        "Rescale or ModSwitch encountered, but "
        "ReductionLogExpander uses m_scale as"
        " a proxy for level and assumes rescaling has not "
        "been performed yet.");
    return;
  }

  // Merge node scale
  if (node->numOperands() == 0) {
    m_scale[node] = node->get<EncodeAtScaleAttr>();
  } else if (node->m_op_type == OpType::Mul) {
    // Accumulate multiplication scale
    m_scale[node] = std::accumulate(
        node->getOperands().begin(), node->getOperands().end(), 0,
        [&](auto &sum, auto &operand) { return sum + m_scale.at(operand); });
  } else {
    // Take the maximum for addition scale
    m_scale[node] =
        std::accumulate(node->getOperands().begin(), node->getOperands().end(),
                        0, [&](auto &max, auto &operand) {
                          return std::max(max, m_scale.at(operand));
                        });
  }

  // Combine nodes with more than 3 operands after reduction
  if (isReductionOp(node->m_op_type) && node->numOperands() > 2) {
    // Sort operands
    for (auto &operand : node->getOperands()) {
      auto order = 0;
      if (m_dtype[operand] == DataType::Plain ||
          m_dtype[operand] == DataType::Raw) {
        order = 1;
      } else if (m_dtype[operand] == DataType::Cipher) {
        order = 2 + m_scale.at(operand);
      }
      m_sorted_operands[order].push_back(operand);
    }
    for (auto &op : m_sorted_operands) {
      m_operands.insert(m_operands.end(), op.second.begin(), op.second.end());
    }

    // Combine operands with more than 3
    assert(m_operands.size() >= 2);
    while (m_operands.size() > 2) {
      std::size_t i = 0;
      while ((i + 1) < m_operands.size()) {
        auto &leftOperand = m_operands[i];
        auto &rightOperand = m_operands[i + 1];
        auto new_node =
            m_dag.makeNode(node->m_op_type, {leftOperand, rightOperand});
        m_next_operands.push_back(new_node);
        i += 2;
      }
      if (i < m_operands.size()) {
        assert((i + 1) == m_operands.size());
        m_next_operands.push_back(m_operands[i]);
      }
      m_operands = m_next_operands;
      m_next_operands.clear();
    }

    assert(m_operands.size() == 2);
    node->setOperands(m_operands);

    m_operands.clear();
    m_next_operands.clear();
    m_sorted_operands.clear();
  }
}

}  // namespace iyfc
