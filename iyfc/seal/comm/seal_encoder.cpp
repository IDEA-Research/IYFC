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
#include "seal_encoder.h"
#include <algorithm>

namespace iyfc {

void CkksEncoder::encode(const ValuationType &src,
                         seal::Plaintext &destination) {
  m_encoder.encode(std::get<std::vector<double>>(src), m_parms_id,
                   pow(2.0, m_ckks_scale), destination);
}

void CkksEncoder::decode(const seal::Plaintext &plain,
                         ValuationType &destination) {
  std::vector<double> result;
  m_encoder.decode(plain, result);
  destination = std::move(result);
}

size_t CkksEncoder::getSlotCnt() { return m_encoder.slot_count(); }

void BfvEncoder::encode(const ValuationType &src,
                        seal::Plaintext &destination) {
  m_encoder.encode(std::get<std::vector<int64_t>>(src), destination);
}

void BfvEncoder::decode(const seal::Plaintext &plain,
                        ValuationType &destination) {
  std::vector<int64_t> result;
  m_encoder.decode(plain, result);
  destination = std::move(result);
}

size_t BfvEncoder::getSlotCnt() { return m_encoder.slot_count(); }
}  // namespace iyfc