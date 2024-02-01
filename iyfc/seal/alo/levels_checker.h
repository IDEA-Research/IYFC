
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

#include "dag/iyfc_dag.h"
#include "dag/node_map.h"

namespace iyfc {

/**
 * @class LevelsChecker
 * @brief Checks levels for addition and multiplication operand nodes.
 */
class LevelsChecker {
 public:
  /**
   * @brief LevelsChecker constructor
   * @param [in] g The DAG.
   * @param [in] types Node data types.
   */
  LevelsChecker(Dag &g, NodeMap<DataType> &types);

  void operator()(const NodePtr &node);

  void free(const NodePtr &node);

  virtual bool isLevelAddOp(const OpType &op_code);

 private:
  Dag &m_dag;
  NodeMap<DataType> &m_types;

  // Reverse levels (leaves have 0, roots have max)
  NodeMap<std::size_t> m_levels;

  bool isModSwitchOp(const OpType &op_code);
};

/**
 * @class BfvLevelsChecker
 * @brief Checks levels for addition and multiplication operand nodes, with level evaluation overridden for Bfv.
 */
class BfvLevelsChecker : public LevelsChecker {
 public:
  /**
   * @brief BfvLevelsChecker constructor
   * @param [in] g The DAG.
   * @param [in] types Node data types.
   */
  BfvLevelsChecker(Dag &g, NodeMap<DataType> &types)
      : LevelsChecker(g, types) {}

  virtual bool isLevelAddOp(const OpType &op_code);
};

}  // namespace iyfc
