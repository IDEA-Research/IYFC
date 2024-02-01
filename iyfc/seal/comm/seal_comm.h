/*
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

#include <cassert>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>
#include "comm_include.h"
#include "dag/iyfc_dag.h"
#include "seal/alo/ckks_parameters.h"
#include "seal/alo/seal_signature.h"
#include "seal/alo/bfv_parameters.h"
#include "seal_executor.h"
#include "seal_public.h"
#include "seal_secret.h"

namespace iyfc {

/**
 * @brief SEALSecret deserialize
*/
std::unique_ptr<SEALSecret> deserialize(const msg::SEALSecret &);

/**
 * @brief getSEALContext get seal context
 * @param [in] params EncryptionParameters parameters
 * @return seal::SEALContext
*/
seal::SEALContext getSEALContext(const seal::EncryptionParameters &params);


/**
 * @brief generateKeys Generate key pair.
 * @param [in] abstract_params const CKKSParameters & parameters
 * @return std::tuple<std::unique_ptr<SEALPublic>, std::unique_ptr<SEALSecret>>
*/
std::tuple<std::unique_ptr<SEALPublic>, std::unique_ptr<SEALSecret>>
generateKeys(const CKKSParameters &abstract_params);

/**
 * @brief generateKeys Generate key pair.
 * @param [in] abstract_params const BfvParameters & parameters
 * @return std::tuple<std::unique_ptr<SEALPublic>, std::unique_ptr<SEALSecret>>
*/
std::tuple<unique_ptr<SEALPublic>, unique_ptr<SEALSecret>> generateKeys(
    const BfvParameters &abstract_params);

}  // namespace iyfc