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
#include "openfhe_util.h"
#include "lattice/stdlatticeparms.h"

namespace iyfc {

lbcrypto::Plaintext MakePlaintext(
    lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& context, std::vector<double>& v,
    uint32_t level) {
  if (level != 0) {
    return context->MakeCKKSPackedPlaintext(v, 1, level);
  }
  return context->MakeCKKSPackedPlaintext(v);
}

lbcrypto::Plaintext MakePlaintext(
    lbcrypto::CryptoContext<lbcrypto::DCRTPoly>& context,
    std::vector<int64_t>& v, uint32_t level) {
  return context->MakePackedPlaintext(v);
}

}  // namespace iyfc
