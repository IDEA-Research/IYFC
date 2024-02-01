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
 * @class LazyRelinearizer
 * @brief Relinearization strategy to relinearize only when needed
 */
class LazyRelinearizer {
  Dag &dag;
  NodeMap<DataType> &type;
  NodeMapOptional<std::uint32_t> &scale;
  NodeMap<bool> pending;  // Nodes pending for processing
  std::uint32_t count;
  std::uint32_t countTotal;

  bool isMultiplicationOp(const OpType &op_code);

  bool isRotationOp(const OpType &op_code);

  bool isUnencryptedType(const DataType &type);

  bool areAllOperandsEncrypted(NodePtr &node);

  bool isEncryptedMultOp(NodePtr &node);

 public:
  /**
   * @brief LazyRelinearizer Constructor
   * @param [in] g DAG
   * @param [in] type Node data types
   * @param [in] scale Node scales
   */
  LazyRelinearizer(Dag &g, NodeMap<DataType> &type,
                   NodeMapOptional<std::uint32_t> &scale);

  ~LazyRelinearizer();

  /**
   * @brief Overloaded () function to traverse and add relinearization nodes where needed
   */
  void operator()(NodePtr &node);
};

}  // namespace iyfc
