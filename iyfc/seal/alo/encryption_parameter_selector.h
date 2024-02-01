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

/**
 * @class EncryptionParametersSelector
 * @brief Decision of SEAL encryption parameters based on the longest chain of the DAG's scales array
 */
class EncryptionParametersSelector {
 public:
  /**
   * @brief EncryptionParametersSelector Constructor
   * @param [in] g DAG
   * @param [in] scales Node scales
   * @param [in] types Node data types
   */
  EncryptionParametersSelector(Dag &g, NodeMapOptional<std::uint32_t> &scales,
                               NodeMap<DataType> &types);
  
  /**
   * @brief Overloaded () function to traverse and decide parameters
  */
  void operator()(const NodePtr &node);

  void free(const NodePtr &node);

  std::vector<std::uint32_t> getEncryptionParameters();

  virtual bool isNeedCheckOp(const OpType &op_code);

  virtual uint32_t getNodeParms(const NodePtr &node);

  Dag &m_dag;

 private:
  NodeMapOptional<std::uint32_t> &m_scales;
  NodeMap<std::vector<std::uint32_t>> m_nodes;
  NodeMap<DataType> &types;
};

/**
 * @class EncryptionParametersSelectorForBfv Inherits from EncryptionParametersSelector
 * @brief BFV does not include rescale operations. Overrides isRescaleOp
 */
class EncryptionParametersSelectorForBfv : public EncryptionParametersSelector {
 public:
  /**
   * @brief EncryptionParametersSelector Constructor
   * @param [in] g DAG
   * @param [in] scales Node scales
   * @param [in] types Node data types
   */
  EncryptionParametersSelectorForBfv(Dag &g,
                                     NodeMapOptional<std::uint32_t> &scales,
                                     NodeMap<DataType> &types)
      : EncryptionParametersSelector(g, scales, types) {}

  virtual bool isNeedCheckOp(const OpType &op_code);
  virtual uint32_t getNodeParms(const NodePtr &node);
};
}  // namespace iyfc
