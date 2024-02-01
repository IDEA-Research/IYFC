
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
#include "encryption_parameter_selector.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace iyfc {

EncryptionParametersSelector::EncryptionParametersSelector(
    Dag &g, NodeMapOptional<std::uint32_t> &scales, NodeMap<DataType> &types)
    : m_dag(g), m_scales(scales), m_nodes(g), types(types) {}

void EncryptionParametersSelector::operator()(
    const NodePtr &node) {  // forward pass traversal

  // calculate the coeff_modulus primes needed for each node.
  // Only consider rescalings; currently, we do not consider the primes needed for output values.
  //
  //             In_1:{}   In_2:{}   In_3:{}
  //                  \          \   /
  //                   \          \ /
  //                    \          * MULTIPLY:{}
  //                     \         |
  //                      \        |
  //                       \       * RESCALE:{40}
  //                        \      |
  //                         \     |
  //                          -----* ADD:{40}
  //                               |
  //                               |
  //                           Out_1:{40}

  auto &operands = node->getOperands();

  if (types[node] == DataType::Raw || node->m_op_type == OpType::Encode) {
    return;
  }

  if (operands.size() > 0) {
    auto &parms = m_nodes[node];

    for (auto &operand : operands) {
      auto &op_parms = m_nodes[operand];

      // Choose the longest among all operand parms
      if (op_parms.size() > parms.size()) {
        parms = op_parms;
      }
    }



    // CKKS only processes rescale nodes, ignoring modulus switches,
    // because there always exists a longest path that does not include modulus switches.
    if (isNeedCheckOp(node->m_op_type)) {
      auto new_size = parms.size() + 1;
      // auto divisor = node->get<RescaleDivisorAttr>();
      auto divisor = getNodeParms(node);
      assert(divisor != 0);
      parms.push_back(divisor);
      // LOG(LOGLEVEL::Debug,
      //     "EncryptionParametersSelector , isNeedCheckOp index %u , scale %u , "
      //     "parms size %u",
      //     node->m_index, divisor, parms.size());
      assert(parms.size() == new_size);
    }
  }
}

void EncryptionParametersSelector::free(const NodePtr &node) {
  m_nodes[node].clear();
}

std::vector<std::uint32_t>
EncryptionParametersSelector::getEncryptionParameters() {
  /*
  Get primes for all nodes. For each output node, get the maximum primes,
  then take the maximum value among all outputs. Additionally, add two primes for the output nodes,
  determined by the output scales and the set output ranges.
  */

  std::vector<std::uint32_t> parms;

  std::uint32_t max_output_size = 0;

  std::uint32_t max_parm = 0;

  std::uint32_t max_len = 0;

  // Process the longest chain for output nodes
  for (const auto &entry : m_dag.getOutputs()) {
    auto &output = entry.second;
    //  output_size = range attribute (bits) + the scale (bits)
    auto size = output->get<RangeAttr>();
    size += m_scales[output];

    // Update max_output_size
    if (size > max_output_size) max_output_size = size;

    auto &op_parms = m_nodes[output];

    if (max_len < op_parms.size()) max_len = op_parms.size();

    for (auto &parm : op_parms) {
      if (parm > max_parm) max_parm = parm;
    }
  }

  assert(max_output_size != 0);

  // When max exceeds 60
  if (max_output_size > m_dag.m_scale) {
    max_parm = m_dag.m_scale;
    // Add m_dag.m_scale primes until the condition is satisfied
    while (max_output_size >= m_dag.m_scale) {
      parms.push_back(m_dag.m_scale);
      max_output_size -= m_dag.m_scale;
    }

    if (max_output_size > 0) {
      parms.push_back(std::max(20u, max_output_size));
    }
  } else {
    // In this case, only add one prime
    if (max_output_size > max_parm) max_parm = max_output_size;
    parms.push_back(max_parm);
  }

  // Add the middle longest parms
  for (auto &entry : m_dag.getOutputs()) {
    auto &output = entry.second;
    auto &op_parms = m_nodes[output];

    if (max_len == op_parms.size()) {
      parms.insert(parms.end(), op_parms.rbegin(), op_parms.rend());
      break;
    }
  }

  // "key prime".
  parms.push_back(max_parm);

  return parms;
}

bool EncryptionParametersSelector::isNeedCheckOp(const OpType &op_code) {
  return op_code == OpType::Rescale;
}

uint32_t EncryptionParametersSelector::getNodeParms(const NodePtr &node) {
  return node->get<RescaleDivisorAttr>();
}

bool EncryptionParametersSelectorForBfv::isNeedCheckOp(const OpType &op_code) {
  return op_code == OpType::Mul;
}

// This is just an estimate of the longest multiplication path. More detailed bit calculations are needed later.
uint32_t EncryptionParametersSelectorForBfv::getNodeParms(const NodePtr &node) {
  return m_dag.m_scale/2;
}

}  // namespace iyfc
