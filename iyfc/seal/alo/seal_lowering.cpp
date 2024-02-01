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
#include "seal_lowering.h"

namespace iyfc {

SEALLowering::SEALLowering(Dag &g, NodeMap<DataType> &type)
    : dag(g), type(type) {}

void SEALLowering::operator()(
    NodePtr &node) {  // must only be used with forward pass traversal
                      // p - c  to neg(c) + p
  if (node->m_op_type == OpType::Sub &&
      type[node->operandAt(0)] != DataType::Cipher &&
      type[node->operandAt(1)] == DataType::Cipher) {
    auto negation = dag.makeNode(OpType::Negate, {node->operandAt(1)});
    auto addition = dag.makeNode(OpType::Add, {node->operandAt(0), negation});
    node->replaceAllUsesWith(addition);
  }
}
}  // namespace iyfc
