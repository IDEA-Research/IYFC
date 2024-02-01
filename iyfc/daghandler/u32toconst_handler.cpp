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

#include "u32toconst_handler.h"

namespace iyfc {

U32ToConstant::U32ToConstant(Dag &g, int type) : m_dag(g), m_type(type) {}

void U32ToConstant::operator()(
    NodePtr &node) {  // must only be used with forward pass traversal
  if (node->m_op_type == OpType::U32Constant) {
    uint32_t value = node->get<Uint32ConstAttr>();
    if (m_type == TYPE_DOUBLE) {
      auto new_node = m_dag.makeDenseConstant(vector<double>{double(value)});
      node->replaceAllUsesWith(new_node);
    } else if (m_type == TYPE_INT64) {
      auto new_node = m_dag.makeInt64DenseConstant(vector<int64_t>{int64_t(value)});
      node->replaceAllUsesWith(new_node);
    }
  }
}

}  // namespace iyfc
