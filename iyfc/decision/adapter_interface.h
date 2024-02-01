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
#include <string>

#include "comm_include.h"
#include "dag/iyfc_dag.h"
#include "parameters_interface.h"

namespace iyfc {
/**
 * @class AdapterInterface
 * @brief Adapter pattern base class that includes interfaces for genKeys, encrypt, decrypt, execute, and serialization.
 */
class AdapterInterface {
 private:
  /* data */
 public:
  AdapterInterface(/* args */) {}
  virtual ~AdapterInterface() {}
  virtual int setParaAndSig(
      std::shared_ptr<ParametersInterface> ptr_parameters) = 0;
  /**
   * @brief genKeys
   * @param[in] alo_name Algorithm name used for decision-making
   * @param[in] dag
   * @return ==0 indicates success
   */
  virtual int genKeys(const std::string &alo_name, const Dag &dag) = 0;
  /**
   * @brief encrypt
   * @param[in] inputs Plaintext inputs
   * @param[in] replace Whether to replace the current existing input
   * @return ==0 indicates success
   */
  virtual int encrypt(const Valuation &inputs, bool replace) = 0;
  /**
   * @brief decrypt
   */
  virtual int decrypt() = 0;

  /**
   * @brief execute
   * @return indicates success
   */
  virtual int execute(Dag &dag) = 0;
  std::shared_ptr<Valuation> m_output_de = nullptr;

  /**
   * @brief Serialize algorithm-related paramaters
   * @param [in] serialize_para Serialization options
   * @param [out] str_info Result stored in a string
   * @return ==0 indicates success
   * @todo Too much related duplicate code needs optimization
   */
  virtual int serializeAloInfo(const DagSerializePara &serialize_para,
                               string &str_info) = 0;
  /**
   * @brief Deserialize algorithm-related parameters
   * @return ==0 indicates success
   * @todo Too much related duplicate code needs optimization
   */
  virtual int deserializeAloInfo(const string &str_info) = 0;
  /**
   * @brief Serialize input information
   * @param [out] str_info Result stored in a string
   * @return ==0 indicates success
   * @todo Too much related duplicate code needs optimization
   */
  virtual int serializeInputInfo(string &str_info) = 0;
  /**
   * @brief Deserialize input information
   * @param [in] str_info Input information as a string
   * @param [in] replace Whether to replace the current adapter's existing input
   * @return ==0 indicates success
   * @todo Too much related duplicate code needs optimization
   */
  virtual int deserializeInputInfo(const string &str_info, bool replace) = 0;
  /**
   * @brief Serialize output information
   * @return ==0 indicates success
   * @todo Too much related duplicate code needs optimization
   */
  virtual int serializeOutputInfo(string &str_info) = 0;
  /**
   * @brief Deserialize output information
   * @return ==0 indicates success
   * @todo Too much related duplicate code needs optimization
   */
  virtual int deserializeOutputInfo(const string &str_info) = 0;

  /**
   * @brief serializeBootstrappingKey
   */
  virtual int serializeBootstrappingKey(std::ostream &stream) {
    // OpenFHE CKKS supports bootstrapping, needs to be overridden for this function
    throw std::logic_error("the alo not support bootstrapping!");
  }

  /**
   * @brief deserializeBootstrappingKey
   */
  virtual int deserializeBootstrappingKey(std::istream &stream) {
    throw std::logic_error("the alo not support bootstrapping!");
  }
};
}  // namespace iyfc