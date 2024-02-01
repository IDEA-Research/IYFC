
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
#include <seal/util/hestdparms.h>

#include <cstdint>

#include "bfv_config.h"
#include "bfv_parameters.h"
#include "daghandler/clean_node_handler.h"
#include "daghandler/constant_handler.h"
#include "daghandler/reduction_handler.h"
#include "daghandler/traversal_handler.h"
#include "daghandler/type_handler.h"
#include "daghandler/u32toconst_handler.h"
#include "decision/parameters_interface.h"
#include "encode_inserter.h"
#include "encryption_parameter_selector.h"
#include "lazy_relinearizer.h"
#include "levels_checker.h"
#include "mod_switcher.h"
#include "parameter_checker.h"
#include "scales_checker.h"
#include "seal_lowering.h"
#include "seal_signature.h"
#include "util/logging.h"

namespace iyfc {

/**
 * @class BfvParametersHandler
 * @brief Inherits from ParametersInterface. Handles SEAL BFV parameter decision and DAG translation logic.
 */
class BfvParametersHandler : public ParametersInterface {
 private:
  BFVConfig m_config;

   /**
   * @brief Process DAG nodes
   * @param [in] dag DAG
   * @param [out] types Node data types
   * @param [out] scales Node scales
   */
  void transform(Dag &dag, NodeMap<DataType> &types,
                 NodeMapOptional<std::uint32_t> &scales);

  /**
   * @brief Verify that the processed DAG is logical
   * @param [in] dag DAG
   * @param [out] types Node data types
   * @param [out] scales Node scales
   */
  void validate(Dag &dag, NodeMap<DataType> &types,
                NodeMapOptional<std::uint32_t> &scales);

  /**
   * @brief Decision degree parameter
   * @param [in] MaxBitsFun Function related to security strength
   * @param [in] bit_count Total bit count required for DAG execution
   */
  std::size_t getMinDegreeForBitCount(int (*MaxBitsFun)(std::size_t),
                                      int bit_count);

  /**
   * @brief Determine encryption parameters
   * @param [in] dag DAG
   * @param [in] scales Node scales
   * @param [in] types Node data types
   */
  int determineEncryptionParameters(Dag &dag,
                                    NodeMapOptional<std::uint32_t> &scales,
                                    NodeMap<DataType> types);

  void extractSignature(const Dag &dag);

  /**
   * @brief transpile
   */
  int transpile(Dag &input_dag);

 public:
  /**
   * @brief Default constructor for BfvParametersHandler
   */
  BfvParametersHandler() {}
  virtual ~BfvParametersHandler() {}
  /**
   * @brief Constructor for BfvParametersHandler
   * @param [in] config Configuration information
   */
  BfvParametersHandler(BFVConfig config) : m_config(config) {}

  /**
   * @brief dagTranspile Entry point for DAG transpilation
   */
  virtual int dagTranspile(Dag &input_dag);

  std::shared_ptr<BfvParameters> m_enc_params = nullptr;
  std::shared_ptr<SealSignature> m_signature = nullptr;
};

}  // namespace iyfc
