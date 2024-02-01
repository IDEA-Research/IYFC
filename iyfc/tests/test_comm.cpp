
/*
*
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
#include "test_comm.h"

using namespace std;
using namespace iyfc;
namespace iyfctest {

Valuation execute(Valuation& inputs, DagPtr dag, Expr& out_expr) {
  setOutput(dag, "test_out", out_expr);
  compileDag(dag);
  genKeys(dag);
  encryptInput(dag, inputs);
  exeDag(dag);
  Valuation outputs;
  decryptOutput(dag, outputs);
  return outputs;
}

template <typename T>
void check_result(const Valuation& output, const vector<T>& vec_out,
                  double precision) {
  // Compare plaintext and ciphertext results
  for (auto& item : output) {
    if (holds_alternative<vector<T>>(item.second)) {
      auto& v = get<vector<T>>(item.second);
      if (v.size() == vec_out.size()) {
        for (uint32_t i = 0; i < v.size(); i++) {
          EXPECT_NEAR(v[i], vec_out[i], precision) << "Incorrect out " << v[i]
                                                   << "," << vec_out[i];
          break;
        }
      } else {
        FAIL() << "err input size"
               << "v.size() = " << v.size() << " and "
               << "vec_out.size() =" << vec_out.size();
      }
    } else {
      FAIL();
    }
    break;
  }
}

template void check_result<double>(const Valuation& output,
                                   const vector<double>& vec_out,
                                   double precision);

template void check_result<int64_t>(const Valuation& output,
                                    const vector<int64_t>& vec_out,
                                    double precision);
}