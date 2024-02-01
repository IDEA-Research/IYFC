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
#include "dag/iyfc_dag.h"
#include "dag/node_map.h"

namespace iyfc {

/**
 * @class EncodeInserter
 * @brief Insert encode nodes to adapt to SEAL execution logic.
 */
class EncodeInserter {
  Dag &dag;
  NodeMap<DataType> &type;
  NodeMapOptional<std::uint32_t> &scale;

  bool isRawType(const DataType &type);
  bool isCipherType(const DataType &type);
  bool isAdditionOp(const OpType &op_code);

  auto insertEncodeNode(OpType op, const NodePtr &other, const NodePtr &node);

 public:
  /**
   * @brief EncodeInserter constructor.
   * @param dag  The DAG.
   * @param type  NodeMap representing node data types.
   * @param scale  NodeMapOptional representing node scales.
   */
  EncodeInserter(Dag &dag, NodeMap<DataType> &type,
                 NodeMapOptional<std::uint32_t> &scale);

  /**
   * @brief Operator overloading for traversing the graph and inserting encode nodes.
   */
  void operator()(NodePtr &node);
};

}  // namespace iyfc
