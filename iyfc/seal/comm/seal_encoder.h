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

#include <variant>

#include "comm_include.h"

namespace iyfc {
// seal encode ckks bfv parameters

/**
 * @class SealEncoderBase base class
 * @brief  seal lib encode interface
 */
class SealEncoderBase {
 public:
  /**
   * @brief  SealEncoderBase default constructor
   */
  SealEncoderBase() {}
  virtual ~SealEncoderBase() {}

  /**
   * @brief  setEncodePara set parameters
   * @param [in] scale Node scale
   * @param [in] parms_id Distinguish between algorithm types
   */
  void setEncodePara(double scale, seal::parms_id_type parms_id) {
    m_ckks_scale = scale;
    m_parms_id = parms_id;
  }

  /**
   * @brief  encode
   * @param [in] src const ValuationType &
   * @param [out] destination seal::Plaintext &
   */
  virtual void encode(const ValuationType &src,
                      seal::Plaintext &destination) = 0;
  /**
   * @brief  decode
   * @param [in] plain const seal::Plaintext &
   * @param [out] destination ValuationType &
   */
  virtual void decode(const seal::Plaintext &plain,
                      ValuationType &destination) = 0;
  virtual size_t getSlotCnt() = 0;
  double m_ckks_scale = 0.0;
  seal::parms_id_type m_parms_id;
};

/**
 * @class CkksEncoder
 * @brief  seal library ckks algorithm encoder, inherits SealEncoderBase, rewrites the encode decode function
 */
class CkksEncoder : public SealEncoderBase {
 public:
  virtual void encode(const ValuationType &src, seal::Plaintext &destination);
  virtual void decode(const seal::Plaintext &plain, ValuationType &destination);
  virtual size_t getSlotCnt();
  CkksEncoder(const seal::SEALContext &context) : m_encoder(context) {}
  ~CkksEncoder() {}

 private:
  seal::CKKSEncoder m_encoder;
};

/**
 * @class BfvEncoder
 * @brief  seal library bfv algorithm encoder, inherits SealEncoderBase, rewrites the encode decode function
 */
class BfvEncoder : public SealEncoderBase {
 public:
  virtual void encode(const ValuationType &src, seal::Plaintext &destination);
  virtual void decode(const seal::Plaintext &plain, ValuationType &destination);
  virtual size_t getSlotCnt();
  BfvEncoder(const seal::SEALContext &context) : m_encoder(context) {}
  ~BfvEncoder() {}

 private:
  seal::BatchEncoder m_encoder;
};

}  // namespace iyfc