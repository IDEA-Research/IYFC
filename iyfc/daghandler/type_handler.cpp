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
#include "type_handler.h"

namespace iyfc {

TypeHandler::TypeHandler(Dag &g, NodeMap<DataType> &types)
    : m_dag(g), m_types(types) {}

void TypeHandler::operator()(NodePtr &node) {  // forward pass
  auto &operands = node->getOperands();
  if (operands.size() > 0) {
    DataType inferred = DataType::Raw;
    for (auto &operand : operands) {
      if (m_types[operand] == DataType::Cipher) inferred = DataType::Cipher;
    }
    if (node->m_op_type == OpType::Encode) {
      m_types[node] = DataType::Plain;
    } else {
      m_types[node] = inferred;
    }
  } else if (node->m_op_type == OpType::Constant ||
             node->m_op_type == OpType::U32Constant) {
    m_types[node] = DataType::Raw;
  } else {
    m_types[node] = node->get<TypeAttr>();
  }
}

}  // namespace iyfc
