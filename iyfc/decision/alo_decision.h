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
#include <unordered_map>
#include <vector>

#include "dag/iyfc_dag.h"
#include "fhe_manager.h"
#include "proto/iyfc.pb.h"

namespace iyfc {

/**
 * @class AloDecision
 * @brief Library, Algorithm, and Parameter Decision Class
 */
class AloDecision {
 public:
  AloDecision();
  ~AloDecision() {}
  /**
   * @brief Decide on the library and algorithm to be used in the decision DAG
   * @param [in] dag
   */
  int deLibAndAlo(Dag &dag);

  /**
   * @brief Decide on the library and algorithm to be used for the entire decision groupdag
   * @param [in] root_dag Groupdag
   * @param [in] m_name2dag Map of group's subgraphs
   */
  int deGroupLibAndAlo(Dag &root_dag,
                       std::unordered_map<std::string, DagPtr> &m_name2dag);
  /**
   * @brief Determine the specific parameters for the algorithm
   * @param [in] alo_name Name of the library and algorithm
   * @param [in] dag
   */
  int dePar(const string &alo_name, Dag &dag);

  /**
   * @brief Encrypt
   * @param [in] inputs Plaintext input
   * @param [in] replace Whether to fully replace the existing input
   */
  int encryptInput(const Valuation &inputs, bool replace = false);

  /**
   * @brief Execute
   */
  int executor(Dag &dag);

  /**
   * @brief Decrypt the result
   */
  int getDecryptOutput(Valuation &output);

  /**
   * @brief Generate all keys
   */
  int genKeys(Dag &dag);

  /**
   * @brief Serialization is uniformly defined in the proto directory
   */
  unique_ptr<msg::Alo> serializeAlo(const DagSerializePara &serialize_para)
      const;  // Serialization containing all underlying algorithm dimensions
  int loadAloInfoFromMsg(const msg::Alo &msg);
  unique_ptr<msg::Input> serializeInput() const;
  int loadInputFromMsg(const msg::Input &msg, bool replace = false);
  unique_ptr<msg::Output> serializeOutput() const;
  int loadOutputFromMsg(const msg::Output &msg);
  int serializeByType(SERIALIZE_DATA_TYPE type, std::ostream &stream);
  int deserializeByType(SERIALIZE_DATA_TYPE type, std::istream &stream);
  std::vector<std::string> getLibs() { return m_libs; }

 private:
  uint32_t m_max_mul_dep{0};
  void setAloName(Dag &dag, std::string &tmp_alo_name);

  void InitDagForDecision(Dag &dag);
  std::vector<std::string> m_libs;
  std::shared_ptr<FheManager> m_fhe_manager;
};

}  // namespace iyfc