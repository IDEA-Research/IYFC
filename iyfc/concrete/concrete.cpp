
/*
 * ConcreteSecret is the private key class for the concrete library, including
 * encryption and decryption. Encryption and decryption in the concrete library
 * are performed in the private key environment. ConcretePublic contains the
 * implementation of the execution flow.
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

#include "concrete.h"
#include "util/overloaded.h"
#include "concrete_executor.h"

namespace iyfc {

unique_ptr<ConcreteValuation> ConcreteSecret::encrypt(
    const Valuation &variant_inputs) {

  auto concret_input = make_unique<ConcreteValuation>();

  for (auto &in : variant_inputs) {
        auto name = in.first; 
        // uint8_t
        auto &v = get<uint8_t>(in.second);
        // Currently only supports shortint4.
        if (v > 15) {
          warn("only support uint2(<15) div");
          return {};
        }
        if (!m_client_key) {
          warn("encrypt m_client_key null ");
          return {};
        }
        (*concret_input)[name] =
            std::make_shared<ConcreteCipher>(c_try_encrypt(m_client_key, v));
      }
  return concret_input;
}

std::unique_ptr<Valuation> ConcreteSecret::decrypt(
    const ConcreteValuation &enc_outputs) {
  auto outputs = std::make_unique<Valuation>();
  for (auto &out : enc_outputs) {
    auto name = out.first;
    visit(Overloaded{[&](const std::shared_ptr<ConcreteCipher> &cipher) {
                       if (!cipher->m_fhe_value) {
                         warn("m_fhe_value null");
                         return;
                       }
                       if (!m_client_key) {
                         warn("m_client_key null");
                         return;
                       }
                       uint8_t value =
                           c_decrypt(m_client_key, cipher->m_fhe_value);
                       (*outputs)[name] = value;
                     },
                     [&](const uint8_t &plain) { (*outputs)[name] = plain; }},
          out.second);
  }
  return outputs;
}

std::unique_ptr<ConcreteValuation> ConcretePublic::execute(
    Dag &dag, const ConcreteValuation &inputs) {
  DagTraversal dag_traverse(dag);

  auto executor = ConcreteExecutor(dag, m_server_key);
  executor.setInputs(inputs);
  dag_traverse.forwardPass(executor);

  auto en_out = std::make_unique<ConcreteValuation>();

  executor.getOutputs(*en_out);
  return en_out;
}

std::tuple<std::unique_ptr<ConcretePublic>, std::unique_ptr<ConcreteSecret>>
concreteGenerateKeys() {
  // C function
  ClientKey *client_key = c_generate_client_key();
  ServerKey *server_key = c_generate_server_key(client_key);
  auto secretCtx = make_unique<ConcreteSecret>(client_key);
  auto publicCtx = make_unique<ConcretePublic>(server_key);

  return make_tuple(move(publicCtx), move(secretCtx));
}

};  // namespace iyfc