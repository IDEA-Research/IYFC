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
#include <map>
#include <numeric>

#include "dag/iyfc_dag.h"
#include "dag/node_map.h"

namespace iyfc {
template <typename>
class NodeMapOptional;
template <typename>
class NodeMap;
/*
 *e.g. (a * (b * (c * d))) => (a * b * c * d) => (a * b) * (c * d)
 */

/**
 * @class Reduction
 * @brief Reduce the height of the tree
 * @details Combines consecutive identical add and multiply nodes in the middle of the DAG to reduce tree height
 *
 *       +
 *     /  \
 *   +   t5(c)    =>           +
 *   /    \                 /  \  \
 *  t1(c)  t2(c)        t1   t2   t5
 */
class Reduction {
  Dag &m_dag;
  bool isReductionOp(const OpType &op_code);

 public:
  Reduction(Dag &g);

  void operator()(NodePtr &node);
};

/**
 * @class ReductionLogExpander
 * @brief Expands nodes with more than 3 operands after reduction operations into nodes that can be processed
 */
class ReductionLogExpander {
  Dag &m_dag;
  NodeMap<DataType> &m_dtype;
  NodeMapOptional<int> m_scale;
  std::vector<NodePtr> m_operands, m_next_operands;
  std::map<std::uint32_t, std::vector<NodePtr>> m_sorted_operands;

  bool isReductionOp(const OpType &op_code);

 public:
  ReductionLogExpander(Dag &g, NodeMap<DataType> &type);

  void operator()(NodePtr &node);
};

}  // namespace iyfc
