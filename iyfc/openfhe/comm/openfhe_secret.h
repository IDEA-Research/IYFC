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
#include "comm_include.h"
#include "openfhe.h"
#include "openfhe/alo/openfhe_signature.h"
#include "openfhe_valuation.h"
#include "util/overloaded.h"

using namespace lbcrypto;

namespace iyfc {

/**
 * @class OpenFheSecret
 * @brief OpenFheSecret class that includes the decrypt operation.
 */
class OpenFheSecret {
 public:
  /**
   * @brief Constructor for OpenFheSecret.
   * @param [in] ctx OpenFheContext.
   * @param [in] sk OpenFhePrivateKey.
   */
  OpenFheSecret(OpenFheContext ctx, OpenFhePrivateKey sk)
      : m_context(ctx), m_secret_key(sk) {}

  /**
   * @brief Decrypt function for OpenFhe.
   * @param [in] enc_outputs Encrypted output.
   * @param [in] signature OpenFheSignature.
   * @return Valuation Decrypted plaintext data.
   */
  template <typename T>
  Valuation decrypt(const OpenFheValuation &enc_outputs,
                    const OpenFheSignature &signature) {
    Valuation outputs;

    for (auto &out : enc_outputs) {
      auto name = out.first;
      visit(Overloaded{[&](const OpenFheCiphertext &cipher) {
                         OpenFhePlaintext plain;
                         m_context->Decrypt(m_secret_key, cipher, &plain);
                         // Decryption includes decoding logic
                         // cout << "plain decrypt " << plain << endl;
                         LOG(LOGLEVEL::Debug, "in decrypt cipher name %s",
                             name.c_str());
                         // bfv encoding
                         if (plain->GetEncodingType() == PACKED_ENCODING)
                           outputs[name] = plain->GetPackedValue();
                         else  // ckks -encoding
                           outputs[name] = plain->GetRealPackedValue();
                       },
                       [&](const OpenFhePlaintext &plain) {
                         // todo
                         outputs[name] = plain->GetRealPackedValue();
                       },
                       [&](const std::shared_ptr<ConstantValue<double>> &raw) {
                         std::vector<double> temp_vec;
                         outputs[name] =
                             raw->expand(temp_vec, signature.batch_size);
                       },
                       [&](const std::shared_ptr<ConstantValue<int64_t>> &raw) {
                         std::vector<int64_t> temp_vec;
                         outputs[name] =
                             raw->expand(temp_vec, signature.batch_size);
                       }},
            out.second);
      auto &v = std::get<vector<T>>(outputs.at(name));
      v.resize(signature.batch_size);
    }
    return outputs;
  }

 private:
  OpenFheContext m_context;
  OpenFhePrivateKey m_secret_key;

  /**
   * @brief OpenFheSecret serialize
   */
  friend std::unique_ptr<msg::OpenFheSecret> serialize(const OpenFheSecret &);
};

/**
 * @brief OpenFheSecret deserialize
 */
std::unique_ptr<OpenFheSecret> deserialize(const msg::OpenFheSecret &);

}  // namespace iyfc