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
#include "examples.h"
using namespace std;
using namespace iyfc;

template <typename T>
void example_simple_expr() {
  std::random_device rd;  
  std::mt19937 gen(rd()); 
  std::uniform_real_distribution<> dis(0.00001, 100000.00);  //[)

  int vec_size = 2048;

  timespec begin_time = gettime();
  DagPtr dag = initDag("hello", vec_size);

  // Set 2.1: Set inputs (supports multiple inputs)
  Expr lhs = setInputName(dag, "lhs");
  Expr rhs = setInputName(dag, "rhs");
  Expr lhs_2 = setInputName(dag, "lhs_2");
  Expr rhs_2 = setInputName(dag, "rhs_2");
  T plain_v = 1;
  setOutput(dag, "fft_out_real", lhs * rhs - lhs_2 * rhs_2 + lhs * plain_v);
  compileDag(dag);
  timespec compile_time = gettime();
  cout << "\n---TIMECOST COMPILE " << time_diff(begin_time, compile_time)
       << " ms-----\n";
  genKeys(dag);

  int cnt = 1;

  // Same key, different inputs can be reused
  while (cnt--) {
    timespec input_time = gettime();

    vector<T> vec_input1;
    vector<T> vec_input2;
    vector<T> vec_input3;
    vector<T> vec_input4;

    for (int i = 0; i < vec_size; i++) {
      vec_input1.emplace_back(dis(gen));
      vec_input2.emplace_back(static_cast<T>(dis(gen)));
      vec_input3.emplace_back(static_cast<T>(dis(gen)));
      vec_input4.emplace_back(static_cast<T>(dis(gen)));
    }
    Valuation inputs{{"lhs", vec_input1},
                     {"rhs", vec_input2},
                     {"lhs_2", vec_input3},
                     {"rhs_2", vec_input4}};
    // Encrypt
    encryptInput(dag, inputs);
    timespec encrept_time = gettime();
    cout << "\n---TIMECOST encryptInput" << time_diff(input_time, encrept_time)
         << " ms-----\n";

    exeDag(dag);
    timespec exe_time = gettime();
    cout << "\n---TIMECOST exeDag " << time_diff(encrept_time, exe_time)
         << " ms-----\n";
    Valuation outputs;
    decryptOutput(dag, outputs);
    printValuation(outputs);
    timespec de_time = gettime();
    cout << "\n---TIMECOST getFFTOutputs " << time_diff(exe_time, de_time)
         << " ms-----\n";
  }
  releaseDag(dag);
}

void example_simple_ckks() { example_simple_expr<double>(); }

void example_simple_bfv() { example_simple_expr<int64_t>(); }