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
#include "seal_comm.h"
#include "seal_encoder.h"
#include "seal_valuation.h"

namespace iyfc {

seal::SEALContext getSEALContext(const seal::EncryptionParameters &params) {
  static unordered_map<seal::EncryptionParameters, seal::SEALContext> map_cache;

  for (auto iter = map_cache.begin(); iter != map_cache.end();) {
    // map_cahe processing
    if (iter->second.key_context_data().use_count() == 2 &&
        iter->first != params) {
      iter = map_cache.erase(iter);
    } else {
      ++iter;
    }
  }

  // find SEALContext
  if (map_cache.count(params) != 0) {
    seal::SEALContext result = map_cache.at(params);
    return result;
  } else {
    auto result = map_cache.emplace(make_pair(
        params, seal::SEALContext(params, true, seal::sec_level_type::none)));
    return result.first->second;
  }
}

tuple<unique_ptr<SEALPublic>, unique_ptr<SEALSecret>> getKeysByContext(
    const seal::SEALContext &context, const vector<int> &vec_rotations,
    std::shared_ptr<SealEncoderBase> p_en) {
  seal::KeyGenerator keygen(context);

  seal::PublicKey public_key;
  seal::GaloisKeys galois_keys;
  seal::RelinKeys relin_keys;

  keygen.create_public_key(public_key);
  keygen.create_galois_keys(vec_rotations, galois_keys);
  keygen.create_relin_keys(relin_keys);

  auto secretCtx = make_unique<SEALSecret>(context, keygen.secret_key(), p_en);
  auto publicCtx = make_unique<SEALPublic>(context, public_key, galois_keys,
                                           relin_keys, p_en);

  return make_tuple(move(publicCtx), move(secretCtx));
}

tuple<unique_ptr<SEALPublic>, unique_ptr<SEALSecret>> generateKeys(
    const CKKSParameters &abstractParams) {
  vector<int> log_qs(abstractParams.prime_bits.begin(),
                     abstractParams.prime_bits.end());

  auto params = seal::EncryptionParameters(seal::scheme_type::ckks);
  params.set_poly_modulus_degree(abstractParams.poly_modulus_degree);
  params.set_coeff_modulus(
      seal::CoeffModulus::Create(abstractParams.poly_modulus_degree, log_qs));

  auto context = getSEALContext(params);
  vector<int> vec_rotations(abstractParams.rotations.begin(),
                            abstractParams.rotations.end());

  std::shared_ptr<SealEncoderBase> p_en =
      std::make_shared<CkksEncoder>(context);
  return getKeysByContext(context, vec_rotations, p_en);
}

tuple<unique_ptr<SEALPublic>, unique_ptr<SEALSecret>> generateKeys(
    const BfvParameters &abstractParams) {
  auto params = seal::EncryptionParameters(seal::scheme_type::bfv);  
  size_t poly_modulus_degree = abstractParams.poly_modulus_degree;
  // size_t poly_modulus_degree = 16384;

  params.set_poly_modulus_degree(poly_modulus_degree);
  params.set_coeff_modulus(
      seal::CoeffModulus::BFVDefault(poly_modulus_degree));
  params.set_plain_modulus(
      seal::PlainModulus::Batching(poly_modulus_degree, 20));
  

  // auto context = getSEALContext(params)
  seal::SEALContext context(params);

  vector<int> vec_rotations(abstractParams.rotations.begin(),
                            abstractParams.rotations.end());

  std::shared_ptr<SealEncoderBase> p_en = std::make_shared<BfvEncoder>(context);

  return getKeysByContext(context, vec_rotations, p_en);
}

}  // namespace iyfc