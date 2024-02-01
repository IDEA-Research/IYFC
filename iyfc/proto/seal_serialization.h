
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
#include <memory>
#include <string>
#include <variant>
#include "seal/comm/seal_comm.h"
#include "seal/comm/seal_public.h"
#include "seal/comm/seal_secret.h"
#include "seal/comm/seal_valuation.h"
#include "util/overloaded.h"
#include "seal/comm/seal_encoder.h"
using namespace std;

namespace iyfc {

using FheObject = msg::FheObject;

template <class T>
auto getSEALTypeTag();

template <>
auto getSEALTypeTag<seal::Ciphertext>();

template <>
auto getSEALTypeTag<seal::Plaintext>();

template <>
auto getSEALTypeTag<seal::SecretKey>();

template <>
auto getSEALTypeTag<seal::PublicKey>();

template <>
auto getSEALTypeTag<seal::GaloisKeys>();

template <>
auto getSEALTypeTag<seal::RelinKeys>();

template <>
auto getSEALTypeTag<seal::EncryptionParameters>();

template <class T>
void serializeSEALType(const T &obj, FheObject *msg);

template <class T>
void deserializeSEALType(T &obj, const FheObject &msg);

template <class T>
void deserializeSEALTypeWithContext(const seal::SEALContext &context, T &obj,
                                    const FheObject &msg);

/**
 * @brief SEALValuation deserialize
*/
unique_ptr<SEALValuation> deserialize(const msg::SEALValuation &msg);
/**
 * @brief SEALValuation serialize
*/
unique_ptr<msg::SEALValuation> serialize(const SEALValuation &obj);

/**
 * @brief SEALPublic serialize
*/
unique_ptr<msg::SEALPublic> serialize(const SEALPublic &obj);

/**
 * @brief SEALPublic deserialize
*/
unique_ptr<SEALPublic> deserialize(const msg::SEALPublic &msg);

/**
 * @brief SEALSecret serialize
*/
unique_ptr<msg::SEALSecret> serialize(const SEALSecret &obj);

/**
 * @brief SEALSecret deserialize
*/
unique_ptr<SEALSecret> deserialize(const msg::SEALSecret &msg);


/**
 * @brief CKKSParameters serialize
*/
unique_ptr<msg::CKKSParameters> serialize(const CKKSParameters &obj) ;
/**
 * @brief CKKSParameters deserialize
*/
unique_ptr<CKKSParameters> deserialize(const msg::CKKSParameters &msg) ;

/**
 * @brief BfvParameters serialize
*/
unique_ptr<msg::BfvParameters> serialize(const BfvParameters &obj) ;

/**
 * @brief BfvParameters deserialize
*/
unique_ptr<BfvParameters> deserialize(const msg::BfvParameters &msg) ;

/**
 * @brief SealSignature serialize
*/
unique_ptr<msg::SealSignature> serialize(const SealSignature &obj) ;
/**
 * @brief SealSignature deserialize
*/
unique_ptr<SealSignature> deserialize(const msg::SealSignature &msg) ;


}  // namespace iyfc
