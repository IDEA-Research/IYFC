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
#include "eager_waterline_rescaler.h"
#include "util/logging.h"

namespace iyfc {

EagerWaterlineRescaler::EagerWaterlineRescaler(
    Dag &g, NodeMap<DataType> &type, NodeMapOptional<std::uint32_t> &scale)
    : Rescaler(g, type, scale) {
  m_min_scale = 0;
  for (auto &source : dag.getSources()) {
    if (scale[source] > m_min_scale) m_min_scale = scale[source];
  }
  m_fixed_rescale = dag.m_scale;
  assert(m_min_scale != 0);
}

void EagerWaterlineRescaler::operator()(
    NodePtr &node) {  //  forward pass 
  if (node->numOperands() == 0) return;  
  if (type[node] == DataType::Raw) {
    handleRawScale(node);
    return;
  }

  if (isRescaleOp(node->m_op_type)) return; 

  if (!isMultiplicationOp(node->m_op_type)) {
    // Non-multiplication, use op[0]
    scale[node] = scale[node->operandAt(0)];
    if (isAdditionOp(node->m_op_type)) {
      // Addition logic
      auto max_scale = scale[node];
      for (auto &operand : node->getOperands()) {
        if (scale[operand] > max_scale) max_scale = scale[operand];
      }

      // Add 1* c to ensure scale correctness constraint + - ==
      for (auto &operand : node->getOperands()) {
        if (scale[operand] < max_scale && type[operand] != DataType::Raw) {
          LOG(LOGLEVEL::Trace,
              "Scaling up t%i from scale %i to match other addition operands "
              "at scale %i",
              operand->m_index, scale[operand], max_scale);

          auto scale_constant = node->m_dag->makeUniformConstant(1);
          dag.updateGroupIndex();
          scale[scale_constant] = max_scale - scale[operand];
          scale_constant->set<EncodeAtScaleAttr>(scale[scale_constant]);

          auto mul_node = dag.makeNode(OpType::Mul, {operand, scale_constant});
          scale[mul_node] = max_scale;
          node->replaceOperand(operand, mul_node);
        }
      }
      for (auto &operand : node->getOperands()) {
        assert(max_scale == scale[operand] || type[operand] == DataType::Raw);
      }
      scale[node] = max_scale;
    }
    return;
  }

  // Multiplication
  std::uint32_t mult_scale = 0;
  for (auto &operand : node->getOperands()) {
    mult_scale += scale[operand];
  }
  assert(mult_scale != 0);
  scale[node] = mult_scale;

  // Rescale if exceeding the waterline during multiplication
  auto temp = node;
  while (mult_scale >= (m_fixed_rescale + m_min_scale)) {
    temp = insertRescale(temp, m_fixed_rescale);
    mult_scale -= m_fixed_rescale;
    assert(mult_scale == scale[temp]);
  }
}

}  // namespace iyfc
