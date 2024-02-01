

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
#include "ckks_rotation_keys_handler.h"

namespace iyfc {

RotationKeys::RotationKeys(Dag &g, NodeMap<DataType> &m_type)
    : m_dag(g), m_type(m_type) {}

void RotationKeys::operator()(const NodePtr &node) {
  auto op = node->m_op_type;

  if (!isLeftRotationOp(op) && !isRightRotationOp(op)) return;
  if (m_type[node] == DataType::Raw) return;
  // Add the rotation count openfhe   Can be reused?
  auto rotation = node->get<RotationAttr>();
  m_keys.insert(isRightRotationOp(op) ? -rotation : rotation);
}

void RotationKeys::free(const NodePtr &node) {
  // No-op
}

std::set<int> RotationKeys::getRotationKeys() {
  return m_keys;
}

bool RotationKeys::isLeftRotationOp(const OpType &op_code) {
  return (op_code == OpType::RotateLeftConst);
}

bool RotationKeys::isRightRotationOp(const OpType &op_code) {
  return (op_code == OpType::RotateRightConst);
}

}  // namespace iyfc
