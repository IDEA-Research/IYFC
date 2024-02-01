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

#include "comm_include.h"
#include "seal/alo/seal_signature.h"
#include "seal_encoder.h"
#include "seal_valuation.h"
#include "util/overloaded.h"

namespace iyfc {

/**
 * @class SEALSecret
 * @brief Class containing decrypt operation for SEAL CKKS.
 */
class SEALSecret {
 public:
 public:
  /**
   * @brief SEALSecret constructor
   * @param[in] ctx SEALContext context
   * @param[in] sk SEAL SecretKey
   * @param[in] p_en std::shared_ptr<SealEncoderBase> Encoder operation
   */
  SEALSecret(seal::SEALContext ctx, seal::SecretKey sk,
             std::shared_ptr<SealEncoderBase> p_en)
      : m_context(ctx),
        m_secret_key(sk),
        encoder_ptr(p_en),
        m_decryptor(ctx, m_secret_key) {}


  /**
   * @brief Decrypt ciphertext outputs.
   * @param [in] enc_outputs Encrypted ciphertext outputs
   * @param [in] signature Signature information
   * @return Valuation Decrypted plaintext outputs
  */
  template <typename T>
  Valuation decrypt(const SEALValuation &enc_outputs,
                    const SealSignature &signature) {
    Valuation outputs;
    ValuationType decode_vec;

    for (auto &out : enc_outputs) {
      auto name = out.first;
      visit(Overloaded{[&](const seal::Ciphertext &cipher) {
                         seal::Plaintext plain;
                         m_decryptor.decrypt(cipher, plain);
                         encoder_ptr->decode(plain, decode_vec);
                         outputs[name] = decode_vec;
                       },
                       [&](const seal::Plaintext &plain) {
                         encoder_ptr->decode(plain, decode_vec);
                         outputs[name] = decode_vec;
                       },
                       [&](const std::shared_ptr<ConstantValue<double>> &raw) {
                         std::vector<double> temp_vec;
                         outputs[name] =
                             raw->expand(temp_vec, signature.vec_size);
                       },
                       [&](const std::shared_ptr<ConstantValue<int64_t>> &raw) {
                         std::vector<int64_t> temp_vec;
                         outputs[name] =
                             raw->expand(temp_vec, signature.vec_size);
                       }},
            out.second);
      auto &v = std::get<vector<T>>(outputs.at(name));
      v.resize(signature.vec_size);
    }
    return outputs;
  }

 private:
  seal::SEALContext m_context;
  seal::SecretKey m_secret_key;

  // seal::CkksEncoder m_encoder;
  std::shared_ptr<SealEncoderBase> encoder_ptr;
  seal::Decryptor m_decryptor;

  friend std::unique_ptr<msg::SEALSecret> serialize(const SEALSecret &);
};

std::unique_ptr<SEALSecret> deserialize(const msg::SEALSecret &);

}  // namespace iyfc