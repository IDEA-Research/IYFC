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

#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "dag/iyfc_dag.h"
#include "dag/node_map.h"

namespace iyfc {

class InconsistentParameters : public std::runtime_error {
 public:
  InconsistentParameters(const std::string &msg);
};

/**
 * @class ParameterChecker
 * @brief Parameter checking class
 */
class ParameterChecker {
  NodeMap<DataType> &types;

 public:
  /**
   * @brief Constructor for ParameterChecker
   * @param [in] g Directed Acyclic Graph (DAG)
   * @param [in] types Node data types
   */
  ParameterChecker(Dag &g, NodeMap<DataType> &types);

  void operator()(const NodePtr &node);

  void free(const NodePtr &node);

 private:
  Dag &m_dag;
  NodeMap<std::vector<std::uint32_t>> m_parms;

  bool isModSwitchOp(const OpType &op_code);

  bool isRescaleOp(const OpType &op_code);
};

}  // namespace iyfc
