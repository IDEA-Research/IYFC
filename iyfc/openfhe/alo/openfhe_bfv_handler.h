
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
#include <cstdint>

#include "daghandler/clean_node_handler.h"
#include "daghandler/constant_handler.h"
#include "daghandler/reduction_handler.h"
#include "daghandler/traversal_handler.h"
#include "daghandler/type_handler.h"
#include "daghandler/u32toconst_handler.h"
#include "decision/parameters_interface.h"
#include "openfhe_parameters.h"
#include "openfhe_signature.h"
#include "plaintext_inserter.h"
#include "util/logging.h"

namespace iyfc {

/**
 * @class OpenFheBfvHandler
 * @brief OpenFHE library BFV algorithm parameter decision process.
 * Inherits from ParametersInterface.
 */
class OpenFheBfvHandler : public ParametersInterface {
 private:
  /**
   * @brief Preprocess the nodes for transformation.
   * @param [in] dag The DAG to be transpiled.
   * @param [out] types Node data types.
   * @param [in] scales scales.
   */
  void transform(Dag &dag, NodeMap<DataType> &types,
                 NodeMapOptional<std::uint32_t> &scales);

  void extractSignature(const Dag &dag);
  /**
   * @brief Determine the parameters for the decision process.
   * @param [in] input_dag The DAG to be transpiled.
   * @param [in] types Node data types.
   */
  void determineParameters(Dag &input_dag, NodeMap<DataType> types);

  int transpile(Dag &input_dag);

 public:
  /**
   * @brief Constructor for OpenFheBfvHandler.
   */
  OpenFheBfvHandler() {}
  virtual ~OpenFheBfvHandler() {}

  /**
   * @brief Transpile the DAG.
   * @param [in] input_dag The DAG to be transpiled.
   */
  virtual int dagTranspile(Dag &input_dag);

  std::shared_ptr<OpenFheBfvPara> m_enc_params = nullptr;
  std::shared_ptr<OpenFheSignature> m_signature = nullptr;
};

}  // namespace iyfc
