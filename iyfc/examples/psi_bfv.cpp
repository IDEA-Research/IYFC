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

void example_psi_bfv() {
  timespec begin_time = gettime();
  DagPtr dag = initDag("hello");
  // Input
  Expr input_expr = setInputName(dag, "x");
  // Output
  
  // Consider ri as a random number; it does not affect the result as long as it is between 0 and 1.
  std::default_random_engine random;
  // Choose a small range for the random parameter
  std::uniform_int_distribution<int64_t> dis2(1, 100);

  Expr tmp_expr = Expr(dag, dis2(random));
  // Computation logic
  vector<int64_t> vec(6, 10);
  for (const auto& item : vec) tmp_expr *= (input_expr - item);

  // Set the output
  setOutput(dag, "z", tmp_expr);

  // Decide algorithm, get the key
  compileDag(dag);
  timespec compile_time = gettime();

  cout << "\n---TIMECOST COMPILE " << time_diff(begin_time, compile_time)
       << " ms-----\n";

  genKeys(dag);
  vector<int64_t> in_value = {1,10,100,2,5,6};
  in_value.resize(getVecSize(dag));
  Valuation inputs{{"x", in_value}};
  encryptInput(dag, inputs);
  timespec genkey_time = gettime();

  cout << "\n---TIMECOST GENKEY ENCRPT " << time_diff(compile_time, genkey_time)
       << " ms-----\n";

  exeDag(dag);
  timespec exe_time = gettime();
  cout << "\n---TIMECOST EXE " << time_diff(genkey_time, exe_time)
       << " ms-----\n";
  Valuation outputs;
  decryptOutput(dag, outputs);

  for (const auto& item : outputs) {
    const auto& v = get<std::vector<int64_t>>(item.second);
    for (int i = 0; i < 6; i++) {
      // Get less than
      auto& item_value = v[i];
      cout << "index : " << i << " output value " << item_value << "\n";
      if (item_value ==0 )
        cout << " match item index : " << i << " output value " << item_value
             << "\n";
    }
  }

  releaseDag(dag);

}
