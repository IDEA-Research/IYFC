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
#include "daghandler/traversal_handler.h"
#include "seal/alo/seal_signature.h"
#include "seal_encoder.h"
#include "seal_valuation.h"

namespace iyfc {

/**
 * @class SEALPublic
 * @brief Class containing encrypt and execute operations for SEAL CKKS.
 */
class SEALPublic {
 public:
  /**
   * @brief SEALPublic constructor
   * @param[in] ctx SEALContext context
   * @param[in] pk SEAL PublicKey
   * @param[in] gk SEAL GaloisKeys
   * @param[in] rk SEAL RelinKeys
   * @param[in] p_en std::shared_ptr<SealEncoderBase> Encoder operation
   */
  SEALPublic(seal::SEALContext ctx, seal::PublicKey pk, seal::GaloisKeys gk,
             seal::RelinKeys rk, std::shared_ptr<SealEncoderBase> p_en)
      : context(ctx),
        publicKey(pk),
        galoisKeys(gk),
        relinKeys(rk),
        encryptor(ctx, publicKey),
        evaluator(ctx),
        encoder_ptr(p_en) {}

  /**
   * @brief Encrypt plaintext inputs.
   * @param [in] variant_inputs Plaintext inputs
   * @param [in] signature Signature information
   * @return SEALValuation Encrypted ciphertext
  */
  template <typename T>
  SEALValuation encrypt(const Valuation &variant_inputs,
                        const SealSignature &signature) {
    SEALValuation sealInputs(context);
    size_t slot_count = encoder_ptr->getSlotCnt();
    LOG(LOGLEVEL::Trace, "slot_count %u, sig_vec_size %u", slot_count,
        signature.vec_size);
    if (slot_count < signature.vec_size) {
      warn("Vector size cannot be larger than slot count");
      return sealInputs;
    }
    if (slot_count % signature.vec_size != 0) {
      warn("Vector size must exactly divide the slot count");
      return sealInputs;
    }

    for (auto &in : variant_inputs) {
      auto name = in.first;  // Input name
      std::vector<T> v;
      if (std::holds_alternative<std::vector<T>>(in.second)) {
        v = get<std::vector<T>>(in.second);
      } else if (std::holds_alternative<double>(in.second)) {
        v.insert(v.end(), signature.vec_size, T(get<double>(in.second)));
      } else if (std::holds_alternative<int64_t>(in.second)) {
        v.insert(v.end(), signature.vec_size, T(get<int64_t>(in.second)));
      }

      auto v_size = v.size();
      // TODO remove this check
      if (v_size != signature.vec_size) {
        v.resize(signature.vec_size);
        LOG(LOGLEVEL::Debug,
            "Input size does not match dag vector size, resize");
      }
      auto info = signature.inputs.at(name);
      auto ctx_data = context.first_context_data();

      for (size_t i = 0; i < info.level; ++i) {
        ctx_data = ctx_data->next_context_data();
      }
      if (info.input_type == DataType::Cipher ||
          info.input_type == DataType::Plain) {
        seal::Plaintext plain;

        if (v_size == 1) {
          encoder_ptr->setEncodePara(info.scale, ctx_data->parms_id());
          ValuationType src(v[0]);
          encoder_ptr->encode(src, plain);
          // encoder.encode(v[0], ctx_data->parms_id(), pow(2.0,
          // info.scale),plain);
        } else {
          vector<T> vec(slot_count);
          assert(v_size <= slot_count);
          assert((slot_count % v_size) == 0);
          auto replicas = (slot_count / v_size);

          for (uint32_t r = 0; r < replicas; ++r) {
            for (uint64_t i = 0; i < v_size; ++i) {
              vec[(r * v_size) + i] = v[i];
            }
          }

          encoder_ptr->setEncodePara(info.scale, ctx_data->parms_id());

          ValuationType src = vec;
          encoder_ptr->encode(src, plain);

          // encoder.encode(vec, ctx_data->parms_id(), pow(2.0,
          // info.scale),plain);
        }
        if (info.input_type == DataType::Cipher) {
          seal::Ciphertext cipher;
          encryptor.encrypt(plain, cipher);
          sealInputs[name] = move(cipher);
        } else if (info.input_type == DataType::Plain) {
          sealInputs[name] = move(plain);
        }
      } else {
        sealInputs[name] = std::shared_ptr<ConstantValue<T>>(
            new DenseConstantValue<T>(signature.vec_size, v));
      }
    }

    return sealInputs;
  }

  /**
   * @brief Execute operations on encrypted inputs.
   * @param [in] dag DAG
   * @param [in] inputs Encrypted inputs
   * @return SEALValuation Encrypted outputs
  */
  template <typename T_EXE>
  SEALValuation execute(
      Dag &dag, const SEALValuation &inputs) {
    // Otherwise fall back to singlecore evaluation
    DagTraversal dag_traverse(dag);

    // Executor to handle SEAL operations
    auto seal_executor = T_EXE(encoder_ptr, dag, context, encryptor, evaluator,
                               galoisKeys, relinKeys);
    seal_executor.setInputs(inputs);

    SEALValuation enc_outputs(context);

    dag_traverse.forwardPass(seal_executor);
    if (seal_executor.IsErr()) {
      // empty
      return enc_outputs;
    }
    seal_executor.getOutputs(enc_outputs);

#ifdef GPU_SUPPORT
// Clear memory pool for GPU version
// context.free_gpu_pool();
#endif

    return enc_outputs;
  }

 private:
  seal::SEALContext context;

  seal::PublicKey publicKey;
  seal::GaloisKeys galoisKeys;
  seal::RelinKeys relinKeys;

  seal::Encryptor encryptor;
  seal::Evaluator evaluator;
  // seal::CkksEncoder encoder;
  std::shared_ptr<SealEncoderBase> encoder_ptr;

  friend std::unique_ptr<msg::SEALPublic> serialize(const SEALPublic &);
};  // namespace iyfc

/**
 * SEALPublic deserialize
*/
std::unique_ptr<SEALPublic> deserialize(const msg::SEALPublic &);

}  // namespace iyfc