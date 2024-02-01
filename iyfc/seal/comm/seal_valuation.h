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
#include <seal/seal.h>

#include <unordered_map>
#include <variant>

#include "dag/iyfc_dag.h"
#include "proto/seal.pb.h"

namespace iyfc {

typedef std::variant<seal::Ciphertext, seal::Plaintext,
                     std::shared_ptr<ConstantValue<double>>,
                     std::shared_ptr<ConstantValue<int64_t>>>
    SchemeValue;

/**
 * @class SEALValuation
 * @brief Data type for SEAL valuation.
*/
class SEALValuation {
 public:
  SEALValuation(const seal::EncryptionParameters &params) : params(params) {}
  SEALValuation(const seal::SEALContext &context)
      : params(context.key_context_data()->parms()) {}

  auto &operator[](const std::string &name) { return m_values[name]; }
  auto begin() { return m_values.begin(); }
  auto begin() const { return m_values.begin(); }
  auto end() { return m_values.end(); }
  auto end() const { return m_values.end(); }

  // void traverseHelper(
  //     std::function<void(std::pair<std::string, SchemeValue> &)> func) {
  //   for (auto &item : m_values) {
  //     func(item);
  //   }
  // }

  void addValues(std::unique_ptr<SEALValuation> &p_valuation) {
    // m_values.insert(p_valuation->begin(), p_valuation->end());
    // p_valuation->traverseHelper([&](std::pair<std::string, SchemeValue> &item) {
    //  m_values[item.first] = std::move(item.second);
    // });
    for (auto &item : p_valuation->m_values) {
      m_values[item.first] = std::move(item.second);
    }
  }

  bool isEmpty() { return m_values.empty(); }

 private:
  seal::EncryptionParameters params;  // For deserialization, context content is needed, so keeping it here.
  std::unordered_map<std::string, SchemeValue> m_values;

  friend std::unique_ptr<msg::SEALValuation> serialize(const SEALValuation &);
};

std::unique_ptr<SEALValuation> deserialize(const msg::SEALValuation &);

}  // namespace iyfc