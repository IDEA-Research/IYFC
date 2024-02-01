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
#include <unordered_map>
#include <variant>
#include "dag/iyfc_dag.h"
#include "openfhe.h"

using namespace lbcrypto;

namespace iyfc {

// Consider CKKS CryptoContext<DCRTPoly> and Ciphertext<DCRTPoly> first
using OpenFheCiphertext = lbcrypto::Ciphertext<DCRTPoly>;
using OpenFhePlaintext = lbcrypto::Plaintext;
using OpenFheContext = lbcrypto::CryptoContext<DCRTPoly>;
using OpenFhePublickKey = lbcrypto::PublicKey<DCRTPoly>;
using OpenFhePrivateKey = lbcrypto::PrivateKey<DCRTPoly>;

typedef std::variant<OpenFheCiphertext, OpenFhePlaintext,
                     std::shared_ptr<ConstantValue<double>>,
                     std::shared_ptr<ConstantValue<int64_t>>>
    OpenFheSchemeValue;

/**
 * @class OpenFheValuation
 * @brief Data types related to OpenFhe.
*/
class OpenFheValuation {
 public:
  OpenFheValuation() {}
  auto &operator[](const std::string &name) { return m_values[name]; }
  auto begin() { return m_values.begin(); }
  auto begin() const { return m_values.begin(); }
  auto end() { return m_values.end(); }
  auto end() const { return m_values.end(); }
  void addValues(std::unique_ptr<OpenFheValuation> &p_valuation) {
    for (auto &item : p_valuation->m_values) {
      m_values[item.first] = std::move(item.second);
    }
  }

  bool isEmpty() { return m_values.empty(); }

 private:
  std::unordered_map<std::string, OpenFheSchemeValue> m_values;

  /**
   * @brief OpenFheValuation serialize
  */
  friend std::unique_ptr<msg::OpenFheValuation> serialize(
      const OpenFheValuation &);
};

  /**
   * @brief OpenFheValuation deserialize
  */
std::unique_ptr<OpenFheValuation> deserialize(const msg::OpenFheValuation &);

}  // namespace iyfc