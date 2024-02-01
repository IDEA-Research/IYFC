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
#include "daghandler/traversal_handler.h"
#include "openfhe.h"
#include "openfhe/alo/openfhe_signature.h"
#include "openfhe_util.h"
#include "openfhe_valuation.h"

using namespace lbcrypto;
using namespace std;

namespace iyfc {

/**
 * @class OpenFhePublic
 * @brief OpenFhePublic class that includes encrypt and execute operations.
 */
class OpenFhePublic {
 public:
  /**
   * @brief Constructor for OpenFhePublic.
   * @param [in] ctx OpenFheContext.
   * @param [in] pk OpenFhePublicKey.
   */
  OpenFhePublic(OpenFheContext ctx, OpenFhePublickKey pk)
      : m_context(ctx), m_public_key(pk) {}

  /**
   * @brief Encryption function for OpenFhe.
   * @param [in] variant_inputs Plain input.
   * @param [in] signature OpenFheSignature.
   */
  template <typename T>
  OpenFheValuation encrypt(const Valuation &variant_inputs,
                           const OpenFheSignature &signature) {
    OpenFheValuation inputs;

    for (auto &in : variant_inputs) {
      auto name = in.first;  // Input name
      std::vector<T> v;
      if (std::holds_alternative<std::vector<T>>(in.second)) {
        v = get<std::vector<T>>(in.second);
      } else if (std::holds_alternative<double>(in.second)) {
        v.insert(v.end(), signature.batch_size, T(get<double>(in.second)));
      } else if (std::holds_alternative<int64_t>(in.second)) {
        v.insert(v.end(), signature.batch_size, T(get<int64_t>(in.second)));
      }

      auto v_size = v.size();
      // TODO remove this check
      if (v_size != signature.batch_size) {
        v.resize(signature.batch_size);
        LOG(LOGLEVEL::Info,
            "Input size does not match dag vector size, resize");
      }
      auto info = signature.inputs.at(name);

      if (info.input_type == DataType::Cipher ||
          info.input_type == DataType::Plain) {
        // MakePackedPlaintext
        uint32_t level = 0;
        if (m_use_bootstrapping) {
          // level = m_final_depth - 15;
          // LOG(LOGLEVEL::Info, "Input  MakePlaintext level %u", level);
        }
        // TODO: Confirm whether it needs to be moved to the specified level in advance
        OpenFhePlaintext plain = MakePlaintext(m_context, v, level);

        /*
        if (std::is_same<typename std::decay<T>::type, double>::value) {
          plain = m_context->MakeCKKSPackedPlaintext(v);
        } else if (std::is_same<typename std::decay<T>::type,
                                int64_t>::value) {
          plain = m_context->MakePackedPlaintext(v);
        } else {
          throw std::logic_error("Unsupported input data type");
        }

        */

        if (info.input_type == DataType::Cipher) {
          OpenFheCiphertext cipher = m_context->Encrypt(m_public_key, plain);
          inputs[name] = move(cipher);

        } else if (info.input_type == DataType::Plain) {
          inputs[name] = move(plain);
        }
      } else {
        inputs[name] = std::shared_ptr<ConstantValue<T>>(
            new DenseConstantValue<T>(signature.batch_size, v));
      }
    }

    return inputs;
  }

  /**
   * @brief Execution function for OpenFhe.
   * @param [in] dag The computation DAG.
   * @param [in] inputs OpenFheValuation inputs.
   * @return OpenFheValuation containing the encrypted results of the execution.
   */
  template <typename T_EXE>
  OpenFheValuation execute(Dag &dag, const OpenFheValuation &inputs) {
    DagTraversal dag_traverse(dag);

    // Need to handle
    auto openfhe_executor = T_EXE(dag, m_context, m_final_depth);
    openfhe_executor.setInputs(inputs);
    OpenFheValuation enc_outputs;
    dag_traverse.forwardPass(openfhe_executor);
    if (openfhe_executor.IsErr()) {
      // empty
      return enc_outputs;
    }
    openfhe_executor.getOutputs(enc_outputs);
    return enc_outputs;
  }

  void setUseBootstrapping(bool use_boot) { m_use_bootstrapping = use_boot; }
  void setFinalDepth(uint32_t depth) { m_final_depth = depth; }

 private:
  OpenFheContext m_context;
  OpenFhePublickKey m_public_key;
  bool m_use_bootstrapping{false};
  uint32_t m_final_depth{0};  // Default is 0, use bootstrapping to encrypt to the specified level

  friend std::unique_ptr<msg::OpenFhePublic> serialize(const OpenFhePublic &);
  friend std::unique_ptr<OpenFhePublic> deserialize(const msg::OpenFhePublic &);
  friend void serializeAutomorphismKey(const OpenFhePublic &obj,
                                       std::ostream &stream);
  friend void deserializeAutomorphismKey(const OpenFhePublic &obj,
                                         std::istream &stream);
};  // namespace iyfc

  /**
   * @brief OpenFhePublic deserialize
   * */
std::unique_ptr<OpenFhePublic> deserialize(const msg::OpenFhePublic &);

}  // namespace iyfc