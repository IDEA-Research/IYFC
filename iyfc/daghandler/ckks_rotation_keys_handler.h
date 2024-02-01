

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
#include <set>

#include "dag/iyfc_dag.h"
#include "dag/node_map.h"

namespace iyfc {

/**
 * @class RotationKeys
 * @brief CKKS algorithm rotation keys parameters.
 * @details Associated with the set of shift amounts for rotation operations, including left and right shifts.
 */
class RotationKeys {
 public:
  /**
   * @brief RotationKeys constructor.
   */
  RotationKeys(Dag &g, NodeMap<DataType> &m_type);

  /**
   * @brief Overloaded operator() for traversing the entire DAG and collecting rotation parameters.
   */
  void operator()(const NodePtr &node);

  void free(const NodePtr &node);

  /**
   * @brief Get the set of rotation parameters for the DAG.
   */
  std::set<int> getRotationKeys();

 private:
  Dag &m_dag;
  NodeMap<DataType> &m_type;
  std::set<int> m_keys;

  bool isLeftRotationOp(const OpType &op_code);

  bool isRightRotationOp(const OpType &op_code);
};

}  // namespace iyfc
