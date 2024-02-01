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

#include "seal_adapter.h"

#include <unordered_map>

namespace iyfc {

int SealCkksAdapter::setParaAndSig(
    std::shared_ptr<ParametersInterface> ptr_parameters) {
  // Cast to subclass pointer
  auto ptr_ckks_param =
      std::static_pointer_cast<SealCkksHandler>(ptr_parameters);
  // Get the pointer to the signature
  m_ckks_signature = ptr_ckks_param->m_signature;
  m_ckks_en_params = ptr_ckks_param->m_enc_params;
  return 0;
}

int SealCkksAdapter::genKeys(const string& alo_name, const Dag& dag) {
  if (m_ckks_en_params == nullptr) {
    throw std::logic_error("genKeys ckks_en_params null !");
  }
  m_seal_ctx = generateKeys(*(m_ckks_en_params));
  return 0;
}

int SealCkksAdapter::encrypt(const Valuation& inputs, bool replace) {
  auto& public_ctx = std::get<0>(m_seal_ctx);
  if (public_ctx == nullptr || m_ckks_signature == nullptr) {
    throw std::logic_error("encrypt public_ctx / ckks_signaturenull !");
  }
  /*
  Encrypting multiple results
  There might be cases where input data types are int64, etc., which are not compatible.
  Here, they are uniformly switched to double for copying.
  */
  Valuation new_inputs = inputs;
  for (auto& in : inputs) {
    if (std::holds_alternative<std::vector<int64_t>>(in.second)) {
      auto v = std::get<std::vector<int64_t>>(in.second);
      vector<double> vec_double_inputs(v.size());
      std::transform(v.begin(), v.end(), vec_double_inputs.begin(),
                     [](const int64_t int_value) {
                       return static_cast<double>(int_value);
                     });

      new_inputs[in.first] = vec_double_inputs;
    }
  }
  unique_ptr<SEALValuation> p_valuation = std::make_unique<SEALValuation>(
      public_ctx->encrypt<double>(new_inputs, *m_ckks_signature));

  if (p_valuation->isEmpty()) {
    return SEAL_ENCRYPT_EMPTY_RESULT;
  }
  // Decide whether to replace or merge the input valuation
  if (replace)
    m_ckks_valution = std::move(p_valuation);
  else
    mergeInput(p_valuation);
  return 0;
}

int SealCkksAdapter::decrypt() {
  auto& secret_ctx = std::get<1>(m_seal_ctx);
  if (secret_ctx == nullptr) {
    throw std::logic_error("decrypt seal secret_ctx null !");
  }
  m_output_de = std::make_shared<Valuation>(
      secret_ctx->decrypt<double>(*m_ckks_output_en, *m_ckks_signature));
  if (m_output_de->empty()) {
    return SEAL_DECRYPT_RESULT_EMPTY;
  }
  return 0;
}

int SealCkksAdapter::execute(Dag& dag) {
  auto& public_ctx = std::get<0>(m_seal_ctx);
  if (public_ctx == nullptr) {
    throw std::logic_error("execute public_ctx null !");
  }
  // m_ckks_output_en.reset();

  m_ckks_output_en = std::make_shared<SEALValuation>(
      public_ctx->execute<CkksSealExecutor>(dag, *m_ckks_valution));
  return 0;
}

int SealCkksAdapter::mergeInput(std::unique_ptr<SEALValuation>& p_valuation) {
  if (m_ckks_valution == nullptr) {
    m_ckks_valution = std::move(p_valuation);
  } else {
    m_ckks_valution->addValues(p_valuation);
  }
  return 0;
}

int SealBfvAdapter::setParaAndSig(
    std::shared_ptr<ParametersInterface> ptr_parameters) {
  // Cast to subclass pointer
  auto ptr_param =
      std::static_pointer_cast<BfvParametersHandler>(ptr_parameters);
  // Get the pointer to the signature
  m_signature = ptr_param->m_signature;
  m_en_params = ptr_param->m_enc_params;
  return 0;
}

int SealBfvAdapter::genKeys(const string& alo_name, const Dag& dag) {
  if (m_en_params == nullptr) {
    throw std::logic_error("genKeys bfv_en_params null !");
  }
  m_seal_ctx = generateKeys(*(m_en_params));
  return 0;
}

int SealBfvAdapter::encrypt(const Valuation& inputs, bool replace) {
  auto& public_ctx = std::get<0>(m_seal_ctx);
  if (public_ctx == nullptr || m_signature == nullptr) {
    throw std::logic_error("encrypt public_ctx / ckks_signaturenull !");
  }
  /*
  Encrypting multiple results
  There might be cases where input data types are int64, etc., which are not compatible.
  Here, they are uniformly switched to double for copying.
  */
  Valuation new_inputs = inputs;
  for (auto& in : inputs) {
    if (std::holds_alternative<std::vector<double>>(in.second)) {
      auto v = std::get<std::vector<double>>(in.second);
      vector<int64_t> vec_double_inputs(v.size());
      std::transform(v.begin(), v.end(), vec_double_inputs.begin(),
                     [](const double int_value) {
                       return static_cast<int64_t>(int_value);
                     });

      new_inputs[in.first] = vec_double_inputs;
    }
  }

  unique_ptr<SEALValuation> p_valuation = std::make_unique<SEALValuation>(
      public_ctx->encrypt<int64_t>(new_inputs, *m_signature));
  if (p_valuation->isEmpty()) {
    return SEAL_ENCRYPT_EMPTY_RESULT;
  }
  if (replace)
    m_valution = std::move(p_valuation);
  else
    mergeInput(p_valuation);
  return 0;
}

int SealBfvAdapter::decrypt() {
  auto& secret_ctx = std::get<1>(m_seal_ctx);
  if (secret_ctx == nullptr) {
    throw std::logic_error("decrypt seal secret_ctx null !");
  }
  m_output_de = std::make_shared<Valuation>(
      secret_ctx->decrypt<int64_t>(*m_output_en, *m_signature));
  if (m_output_de->empty()) {
    return SEAL_DECRYPT_RESULT_EMPTY;
  }
  return 0;
}

int SealBfvAdapter::execute(Dag& dag) {
  auto& public_ctx = std::get<0>(m_seal_ctx);
  if (public_ctx == nullptr) {
    throw std::logic_error("execute public_ctx null !");
  }
  m_output_en = std::make_shared<SEALValuation>(
      public_ctx->execute<BfvSealExecutor>(dag, *m_valution));
  return 0;
}

int SealBfvAdapter::mergeInput(std::unique_ptr<SEALValuation>& p_valuation) {
  if (m_valution == nullptr) {
    m_valution = std::move(p_valuation);
  } else {
    m_valution->addValues(p_valuation);
  }
  return 0;
}

}  // namespace iyfc