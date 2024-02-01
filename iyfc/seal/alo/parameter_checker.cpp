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

#include "parameter_checker.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace iyfc {

InconsistentParameters ::InconsistentParameters(const std::string &msg)
    : std::runtime_error(msg) {}

ParameterChecker::ParameterChecker(Dag &g, NodeMap<DataType> &types)
    : types(types), m_dag(g), m_parms(g) {}

void ParameterChecker::operator()(
    const NodePtr &node) {  // forward pass 
  auto &operands = node->getOperands();
  if (types[node] == DataType::Raw || node->m_op_type == OpType::Encode) {
    return;
  }

  if (operands.size() > 0) {
    auto &parms = m_parms[node];

    for (auto &operand : operands) {
      auto &op_parms = m_parms[operand];
      if (op_parms.size() > 0) {
        if (parms.size() > 0) {
          if (op_parms.size() != parms.size()) {
            warn("Two operands require different number of primes");
          }
          for (std::size_t i = 0; i < parms.size(); ++i) {
            if (parms[i] == 0) {
              // If primes are set to 0, it indicates that the node is a modswitch operation.
              // In this case, the actual parameters need to be filled based on the operands.
              parms[i] = op_parms[i];
            } else if (op_parms[i] != 0) {
              // If primes are non-zero, it is expected that the parameters among operands should be equal.
              if (parms[i] != op_parms[i]) {
                warn("Primes required by two operands do not match");
              }
            }
          }
        } else {
          // For nodes with a single operand, parameters are directly passed.
          parms = op_parms;
        }
      }
    }

    if (isModSwitchOp(node->m_op_type)) {
      // modswitch  add 0
      parms.push_back(0);
    } else if (isRescaleOp(node->m_op_type)) {
      //  rescale? add  rescalediv value
      auto divisor = node->get<RescaleDivisorAttr>();
      assert(divisor != 0);
      parms.push_back(divisor);
    }
  } else {
    auto &parms = m_parms[node];
    std::uint32_t level = node->get<EncodeAtLevelAttr>();
    while (level > 0) {
      parms.push_back(0);
      level--;
    }
  }
}

void ParameterChecker::free(const NodePtr &node) { m_parms[node].clear(); }

bool ParameterChecker::isModSwitchOp(const OpType &op_code) {
  return (op_code == OpType::ModSwitch);
}

bool ParameterChecker::isRescaleOp(const OpType &op_code) {
  return (op_code == OpType::Rescale);
}

}  // namespace iyfc
