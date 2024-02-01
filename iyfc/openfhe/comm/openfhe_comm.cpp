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

#include "openfhe_comm.h"

using namespace lbcrypto;
namespace iyfc {

OpenFheContext getContext(
    const lbcrypto::CCParams<CryptoContextCKKSRNS> &params) {
  // Generate a new context each time it's called
  return GenCryptoContext(params);
}

void print_moduli_chain(const DCRTPoly &poly) {
  int num_primes = poly.GetNumOfElements();
  double total_bit_len = 0.0;
  for (int i = 0; i < num_primes; i++) {
    auto qi = poly.GetParams()->GetParams()[i]->GetModulus();
    std::cout << "q_" << i << ": " << qi << ",  log q_" << i << ": "
              << std::log(qi.ConvertToDouble()) / std::log(2) << std::endl;
    total_bit_len += std::log(qi.ConvertToDouble()) / std::log(2);
  }
  std::cout << "Total bit length: " << total_bit_len << std::endl;
}

tuple<unique_ptr<OpenFhePublic>, unique_ptr<OpenFheSecret>> generateKeys(
    const OpenFheCkksPara &params) {
  CCParams<CryptoContextCKKSRNS> parameters;
  uint32_t final_depth = params.mult_depth;
  parameters.SetMultiplicativeDepth(params.mult_depth);
  parameters.SetScalingModSize(params.scaling_mod_size);
  parameters.SetFirstModSize(params.first_mod_size);
  parameters.SetBatchSize(params.batch_size);

  // Use default values
  parameters.SetScalingTechnique(FLEXIBLEAUTO);  // Improve precision
  // parameters.SetScalingTechnique(FIXEDAUTO);
  SecretKeyDist secret_key_dist = UNIFORM_TERNARY;
  parameters.SetSecretKeyDist(secret_key_dist);
  parameters.SetSecurityLevel(HEStd_128_classic);
  parameters.SetKeySwitchTechnique(HYBRID);

  // boot
  std::vector<uint32_t> level_budget = {4, 4};
  std::vector<uint32_t> bsgs_dim = {0, 0};

  if (params.need_bootstrapping) {
    // Estimated number of bootstrapping rounds
    uint32_t num_iterations = params.mult_depth / LEVELS_BEFORE_BOOTSTRAP;
    uint32_t approx_bootstrap_depth = 8 + (num_iterations - 1);
    // uint32_t levels_used_before_bootstrap = 10;
    final_depth = LEVELS_BEFORE_BOOTSTRAP +
                  FHECKKSRNS::GetBootstrapDepth(approx_bootstrap_depth,
                                                level_budget, secret_key_dist);
    parameters.SetMultiplicativeDepth(final_depth);

    LOG(LOGLEVEL::Debug, "total depth set for Bootstrap %u", final_depth);
  }

  CryptoContext<DCRTPoly> context = GenCryptoContext(parameters);
  context->Enable(PKE);
  context->Enable(KEYSWITCH);
  context->Enable(LEVELEDSHE);
  context->Enable(ADVANCEDSHE);

  uint32_t num_slots = params.batch_size;

  if (params.need_bootstrapping) {
    context->Enable(FHE);
    context->EvalBootstrapSetup(level_budget, bsgs_dim, num_slots);
  }

  auto keys = context->KeyGen();
  // stored in cryptocontext
  context->EvalMultKeyGen(keys.secretKey);

  context->EvalRotateKeyGen(keys.secretKey, params.rotations);

  if (params.need_bootstrapping)
    context->EvalBootstrapKeyGen(keys.secretKey, num_slots);

  LOG(LOGLEVEL::Debug, "CKKS scheme is using ring dimension %u",
      context->GetRingDimension());
  const std::vector<DCRTPoly> &ckks_pk = keys.publicKey->GetPublicElements();
  if (logLevelLeast(LOGLEVEL::Debug)) print_moduli_chain(ckks_pk[0]);

  auto public_ctx = std::make_unique<OpenFhePublic>(context, keys.publicKey);
  if (params.need_bootstrapping) {
    public_ctx->setUseBootstrapping(true);
  }
  public_ctx->setFinalDepth(final_depth);

  auto secret_ctx = std::make_unique<OpenFheSecret>(context, keys.secretKey);
  return make_tuple(move(public_ctx), move(secret_ctx));
}

std::tuple<unique_ptr<OpenFhePublic>, unique_ptr<OpenFheSecret>> generateKeys(
    const OpenFheBfvPara &params) {
  CCParams<lbcrypto::CryptoContextBFVRNS> parameters;
  parameters.SetPlaintextModulus(params.plain_modulus);
  parameters.SetMultiplicativeDepth(params.mult_depth);
  parameters.SetScalingModSize(params.scaling_mod_size);
  // parameters.SetMaxRelinSkDeg(); relinear depth
  parameters.SetBatchSize(params.batch_size);  // plain_m % 2*bat = 1

  // Use default values
  SecretKeyDist secret_key_dist = UNIFORM_TERNARY;
  parameters.SetSecretKeyDist(secret_key_dist);
  parameters.SetSecurityLevel(HEStd_128_classic);
  // parameters.SetMultipartyMode(NOISE_FLOODING_MULTIPARTY); //todo --
  // Used when multiple parties participate： parameters.SetMultiplicationTechnique(BEHZ); //todo

  CryptoContext<DCRTPoly> context = GenCryptoContext(parameters);

  LOG(LOGLEVEL::Debug, "gen context");
  context->Enable(PKE);
  context->Enable(KEYSWITCH);
  context->Enable(LEVELEDSHE);
  // context->Enable(ADVANCEDSHE);
  // context->Enable(MULTIPARTY);

  auto keys = context->KeyGen();
  // stored in cryptocontext
  context->EvalMultKeyGen(keys.secretKey);
  context->EvalRotateKeyGen(keys.secretKey, params.rotations);

  LOG(LOGLEVEL::Debug, "bfv scheme is using ring dimension %u",
      context->GetRingDimension());
  const std::vector<DCRTPoly> &ckks_pk = keys.publicKey->GetPublicElements();
  if (logLevelLeast(LOGLEVEL::Debug)) print_moduli_chain(ckks_pk[0]);

  auto public_ctx = std::make_unique<OpenFhePublic>(context, keys.publicKey);
  public_ctx->setFinalDepth(params.mult_depth);

  auto secret_ctx = std::make_unique<OpenFheSecret>(context, keys.secretKey);
  return make_tuple(move(public_ctx), move(secret_ctx));
}
}  // namespace iyfc