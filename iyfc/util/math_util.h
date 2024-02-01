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
#include <fftw3.h>

#include <cmath>
#include <complex>
#include <vector>

namespace iyfc {
using namespace std;
typedef std::complex<double> ComplexDouble;

/**
 * @brief Convert base-10 number to base-k
 */
std::vector<uint32_t> decimalConvert(uint32_t num, uint32_t k,
                                     uint32_t bits = 32);
/**
 * @brief Split an integer into individual digits and reverse, padding with zeros  1234 --> [4,3,2,1,0,0,0,0,0]
 */
void getNumReVec(int num, std::vector<int>& vec_num, size_t total_size = 32);

/**
 * @brief Generate a mask for comparing results   [1,0,0,0,0,0,0,0...1,0,0]
 */
void getMaskVec(uint32_t bits, uint32_t total_size,
                std::vector<double>& vec_mask);
/**
 * @brief Generate a mask for summing values   111110000000000....
 */
void getSumMaskVec(uint32_t bits, uint32_t total_size,
                   std::vector<double>& vec_mask);

/**
 * @brief Get the real part of the result after FFT
 */
double getComplexNum(fftw_complex* vec_complex, int n);

int getLog(int k, int base = 2);

double CalculateApproximationErrorMax(const std::vector<double>& result,
                                      const std::vector<double>& expected);

double CalculateApproximationError(const std::vector<double>& result,
                                   const std::vector<double>& expected);

/**
 * @class FastFourierTransform
 * @brief Helper class for FFT algorithm
 */
class FastFourierTransform {
 public:
  fftw_complex *m_in, *m_out;

  FastFourierTransform(int n, int sign) : N(n) {
    m_in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
    m_out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
    p = fftw_plan_dft_1d(n, m_in, m_out, sign, FFTW_MEASURE);
  }

  void fft() { fftw_execute(p); /* repeat as needed */ }

  ~FastFourierTransform() {
    fftw_destroy_plan(p);
    fftw_cleanup();
    if (m_in != NULL) fftw_free(m_in);
    if (m_out != NULL) fftw_free(m_out);
  }

 private:
  int N;
  fftw_plan p;
};

}  // namespace iyfc

