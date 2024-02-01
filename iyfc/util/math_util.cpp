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

#include "math_util.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

#include "comm_include.h"

namespace iyfc {

std::vector<uint32_t> IYFC_SO_EXPORT decimalConvert(uint32_t num, uint32_t k,
                                                    uint32_t bits) {
  std::vector<uint32_t> vec_result;
  // Start converting
  while (num != 0) {
    vec_result.emplace_back(num % k);
    num = num / k;
  }
  // Fill 0
  vec_result.resize(bits);
  std::reverse(vec_result.begin(), vec_result.end());
  return vec_result;
}

void getNumReVec(int num, std::vector<int>& vec_num, size_t total_size) {
  std::string str_num = std::to_string(num);
  for (const auto& ch : str_num) vec_num.emplace_back(ch - '0');
  std::reverse(vec_num.begin(), vec_num.end());
  vec_num.resize(total_size);
}

void getMaskVec(uint32_t bits, uint32_t total_size,
                std::vector<double>& vec_mask) {
  vec_mask.resize(total_size);

  for (int i = 0; i < total_size; i++) {
    if (i % bits == 0) vec_mask[i] = 1.0;
  }
}

void getSumMaskVec(uint32_t bits, uint32_t total_size,
                   std::vector<double>& vec_mask) {
  vec_mask.insert(vec_mask.begin(), bits, 1);
  vec_mask.resize(total_size);
}

double getComplexNum(fftw_complex* vec_complex, int n) {
  double result = 0.0;
  for (int i = 0; i < n; i++) {
    // N times of intt in. The reason is that the inverse Fourier transform defined here is not divided by N so /n is required
    // result += (int)std::round(vec_complex[i][0] / n) * (int)std::pow(10, i);
    double tmp_num = vec_complex[i][0] / n;
    if (tmp_num > 0.0001) {
      result += tmp_num * std::pow(10, i);
    }
  }
  return result;
}
int getLog(int k, int base) { return int(std::log(k) / std::log(base)); }

double IYFC_SO_EXPORT CalculateApproximationErrorMax(
    const std::vector<double>& result, const std::vector<double>& expected) {
  if (result.size() != expected.size())
    throw std::logic_error(
        "Cannot compare vectors with different numbers of elements");

  // using the infinity norm
  double maxError = 0;
  for (size_t i = 0; i < result.size(); ++i) {
    double error = std::abs(result[i] - expected[i]);
    if (maxError < error) maxError = error;
  }
  printf("maxError diff  %.6f \n", maxError);
  return std::abs(std::log2(maxError));
}

double IYFC_SO_EXPORT CalculateApproximationError(
    const std::vector<double>& result, const std::vector<double>& expected) {
  if (result.size() != expected.size())
    throw std::logic_error(
        "Cannot compare vectors with different numbers of elements");

  // using the Euclidean norm
  double avrg = 0;
  for (size_t i = 0; i < result.size(); ++i) {
    avrg += std::pow(std::abs(result[i] - expected[i]), 2);
  }

  avrg = std::sqrt(avrg) / result.size();  // get the average
  printf("avrg diff %.6f \n", avrg);
  return std::abs(std::log2(avrg));
}

}  // namespace iyfc
