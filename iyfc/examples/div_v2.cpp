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
using namespace iyfc;
using namespace std;

const int P_LEN = 32;
const uint64_t P = pow(2, P_LEN);

void example_div_v2() {
  // ===========================================================================================================
  // Data Preparation Phase: Determine dividend, divisor, modulus, and perform DFT transformation with encoding
  // ===========================================================================================================
  std::random_device
      rd;  // Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd());  // Standard mersenne_twister_engine seeded with rd()
  // std::uniform_real_distribution<> dis(P / 2.0, P);  //[)
  
  // Data close to 0 will have larger errors; data between 50 and 1024 is acceptable
  std::uniform_real_distribution<> dis(0.00001, 100);  //[)

  double num_to_div = dis(gen);

  // Step 0: Build the comparison computation DAG
  DagPtr dag = initDag("division");
  // Step 1: Set precision parameters

  Expr expr_x = setInputName(dag, "x");

  setOutput(dag, "div_ressult", num_to_div / expr_x);
  // =========================================================================================
  // Program Execution Phase: Encrypt input, execute program, decrypt output, compare results
  // =========================================================================================
  // Note 2: Spelling should be compileDag
  compileDag(dag);  // Compile program
  genKeys(dag);     // Generate key environment

  int cnt = 1;
  while (cnt--) {
    double num_origin = dis(gen);

    Valuation inputs1{{"x", num_origin}};

    encryptInput(dag, inputs1);

    // Step 2: Execute the program
    exeDag(dag);

    // Step 3: Decrypt the output
    Valuation outputs;
    decryptOutput(dag, outputs);
    const auto& v = std::get<std::vector<double>>(outputs["div_ressult"]);

    double de_result;
    // cout << "Modulus is: " << P << endl;
    cout << "x为:" << num_origin;
    for (const auto& num : v) {
      de_result = num;
      break;
    }

    double true_result = num_to_div/ num_origin;

    cout << "num_to_div / x calculation result: " << de_result;
    cout << "Ideal division result: " << true_result;

    double binary_precision =
        std::log(std::abs(de_result - true_result)) / std::log(2.0);
    double decimal_precision =
        std::log(std::abs(de_result - true_result)) / std::log(10.0);

    std::cout << "Binary precision: " << binary_precision;
    std::cout << "Decimal precision: " << decimal_precision << std::endl;
  }

  releaseDag(dag); 

}