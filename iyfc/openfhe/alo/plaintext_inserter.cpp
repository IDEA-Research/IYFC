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
#include "plaintext_inserter.h"

namespace iyfc {


  bool PlaintextInserter::isRawType(const DataType &type) { return type == DataType::Raw; }
  bool PlaintextInserter::isCipherType(const DataType &type) { return type == DataType::Cipher; }
  bool PlaintextInserter::isAdditionOp(const OpType &op_code) {
    return ((op_code == OpType::Add) || (op_code == OpType::Sub));
  }

  NodePtr PlaintextInserter::insertPlaintextNode(OpType op, const NodePtr &other, const NodePtr &node) {
    auto new_node = dag.makeNode(OpType::Encode, {node});
    type[new_node] = DataType::Plain;
    if (isAdditionOp(op)) {
      scale[new_node] = scale[other];
    } else {
      scale[new_node] = scale[node];
    }
    new_node->set<EncodeAtScaleAttr>(scale[new_node]);
    return new_node;
  }

PlaintextInserter::PlaintextInserter(Dag &g, NodeMap<DataType> &type,
                 NodeMapOptional<std::uint32_t> &scale)
      : dag(g), type(type), scale(scale) {}

  void PlaintextInserter::operator()(NodePtr &node) { // must only be used with forward pass traversal
    auto &operands = node->getOperands();
    if (operands.size() == 0) return; // inputs

    assert(operands.size() <= 2);
    if (operands.size() == 2) {
      auto &left_op = operands[0];
      auto &right_op = operands[1];
      if (isCipherType(type[left_op]) && isRawType(type[right_op])) {
        auto new_node = insertPlaintextNode(node->m_op_type, left_op, right_op);
        node->replaceOperand(right_op, new_node);
      }

      if (isCipherType(type[right_op]) && isRawType(type[left_op])) {
        auto new_node = insertPlaintextNode(node->m_op_type, right_op, left_op);
        node->replaceOperand(left_op, new_node);
      }
    }
  }

} // namespace iyfc
