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
#include "lazy_relinearizer.h"

namespace iyfc {

bool LazyRelinearizer::isMultiplicationOp(const OpType &op_code) {
  return (op_code == OpType::Mul);
}

bool LazyRelinearizer::isRotationOp(const OpType &op_code) {
  return ((op_code == OpType::RotateLeftConst) ||
          (op_code == OpType::RotateRightConst));
}

bool LazyRelinearizer::isUnencryptedType(const DataType &type) {
  return type != DataType::Cipher;
}

bool LazyRelinearizer::areAllOperandsEncrypted(NodePtr &node) {
  for (auto &op : node->getOperands()) {
    assert(type[op] != DataType::Undef);
    if (isUnencryptedType(type[op])) {
      return false;
    }
  }
  return true;
}

bool LazyRelinearizer::isEncryptedMultOp(NodePtr &node) {
  return (isMultiplicationOp(node->m_op_type) && areAllOperandsEncrypted(node));
}

LazyRelinearizer::LazyRelinearizer(Dag &g, NodeMap<DataType> &type,
                                   NodeMapOptional<std::uint32_t> &scale)
    : dag(g), type(type), scale(scale), pending(g) {
  count = 0;
  countTotal = 0;
}

LazyRelinearizer::~LazyRelinearizer() {
}

void LazyRelinearizer::operator()(
    NodePtr &node) {  // must only be used with forward pass traversal
  auto &operands = node->getOperands();
  if (operands.size() == 0) return;  // inputs

  bool delayed = false;

  if (isEncryptedMultOp(node)) {
    assert(pending[node] == false);
    pending[node] = true;
    delayed = true;
  } else if (pending[node] == false) {
    return;
  }

  bool must_insert = false;
  assert(node->numUses() > 0);

  auto first_use = node->getUses()[0];
  for (auto &use : node->getUses()) {
    if (isEncryptedMultOp(use) || isRotationOp(use->m_op_type) ||
        use->m_op_type == OpType::Output ||
        (first_use != use)) {  // different uses
      must_insert = true;
      break;
    }
  }

  if (must_insert) {
    auto relin_node = dag.makeNode(OpType::Relinearize, {node});
    ++countTotal;

    type[relin_node] = type[node];
    scale[relin_node] = scale[node];
    node->replaceOtherUsesWith(relin_node);
  } else {
    if (delayed) ++count;
    for (auto &use : node->getUses()) {
      pending[use] = true;
    }
  }
}

}  // namespace iyfc
