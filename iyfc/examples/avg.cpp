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

/**
 * @brief Example demonstrating the average calculation using group_dag.
 * @details
 * Preconditions:
 * 1. This solution is only used for averaging and calculating variance.
 * 2. The maximum divisor is known to be 1024.
 *
 * Specific steps:
 * A mid-layer application is required for an additional request, and data parties assist in the calculation.
 * 1. The calculation party first calculates the ciphertext cnt and multiplies it by a random number r. The ciphertext c1 * r is sent to the data party.
 * 2. The data party decrypts c1 * r, performs plaintext calculation 1 / decrypt(c1 * r), and encrypts the result before sending it to the calculation party.
 * 3. The calculation party calculates (1 / c1 * r) * r to obtain the ciphertext 1 / c1 result.
 */
void example_avg() {
  // Initialization: Build a group_dag containing cnt_dag and avg_dag

  DagPtr group = initDagGroup("group");
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(1.0, 1024.0);
  // FFT * random: precision is limited, random must be an integer
  double random = dis(gen);

  // Dag 1: Counting
  DagPtr dag_cnt_sum = initDag("child_dag_cnt");
  Expr lhs = setInputName(dag_cnt_sum, "lhs");
  Expr rhs = setInputName(dag_cnt_sum, "rhs");
  // Currently, use seal for a single comparison condition.
  // If there are more than two conditions, switch to the openfhe library.
  setOutput(dag_cnt_sum, "cnt", random * QueryCnt(lhs < rhs || lhs == rhs));
  addDag(group, dag_cnt_sum);
  
  // Dag 2: Average
  DagPtr dag_avg = initDag("child_dag_avg");
  setNextNodeIndex(dag_avg, getNextNodeIndex(group));
  Expr inverse_cnt = setInputName(dag_avg, "inverse_cnt");
  Expr lhs_avg = setInputName(dag_avg, "lhs_avg");
  Expr rhs_avg = setInputName(dag_avg, "rhs_avg");
  Expr fft_real = setInputName(dag_avg, "fft_real");
  Expr fft_imag = setInputName(dag_avg, "fft_imag");
  setOutput(dag_avg, "fft_out_real",
            random * inverse_cnt * (QuerySum(fft_real, lhs_avg < rhs_avg || lhs_avg == rhs_avg) ));
  setOutput(dag_avg, "fft_out_imag",
            random * inverse_cnt * (QuerySum(fft_imag, lhs_avg < rhs_avg || lhs_avg == rhs_avg)));
  addDag(group, dag_avg);
  compileDag(group);

  // After compilation, only contains genkey + sig information for the client to generate keys
  setDagSerializePara(group, false, true, true, false, false, false);
  std::string str_genkeys_info;
  saveDagToStr(group, str_genkeys_info);

  // Step 1.0: Client generates keys
  DagPtr dag_for_gen_keys = loadDagFromStr(str_genkeys_info);
  genKeys(dag_for_gen_keys);
  // Step 1.1: Group obtains public key
  setDagSerializePara(dag_for_gen_keys, false, false, false, true, true, false);
  std::string str_only_for_exe;
  saveKeysInfoToStr(dag_for_gen_keys, str_only_for_exe);
  loadKeysFromStr(group, str_only_for_exe);

  int test_cnt = 2;
  while (test_cnt--) {
    // Contruct test data
    vector<uint32_t> vec_input1;
    vector<uint32_t> vec_input2;
    vector<uint32_t> vec_org;
    int true_cnt = 0;
    int true_sum = 0;
    for (int i = 0; i < MAX_CMP_NUM; i++) {
      uint32_t lhs_1 = rand() % MAX_CMP_NUM + 950;
      uint32_t rhs_1 = rand() % MAX_CMP_NUM;
      uint32_t org = rand() % MAX_CMP_NUM;
      vec_input1.emplace_back(lhs_1);
      vec_input2.emplace_back(rhs_1);
      vec_org.emplace_back(org);
      if (lhs_1 < rhs_1) {
        true_cnt++;
        true_sum += org;
      }
    }

    // Step 2: Execute ciphertext counting
    /* Simplifying, in this round, the data is encrypted by the server itself;
    it can also be encrypted by the client and then transmitted.
    */
    Valuation inputs;
    encodeOrgInputforCmp(vec_input1, "lhs", inputs);
    encodeOrgInputforCmp(vec_input2, "rhs", inputs);
    encryptInput(dag_cnt_sum, inputs, true);
    exeDag(dag_cnt_sum);
    std::string str_output_cnt;
    savaOutputTostr(dag_cnt_sum, str_output_cnt);

    // Step 3: Client retrieves cnt execution result and performs plaintext calculation: random / count
    loadOutputFromStr(dag_for_gen_keys, str_output_cnt);
    double cnt;
    getCntRandomOutput(dag_for_gen_keys, "cnt", cnt);
    double cnt_mul=  1.0/cnt;
    printf(
        "success get cnt = %lf, mul random %lf, cal_cnt %lf, true_cnt %d, cnt_mul %lf \n",
        cnt, random, cnt / random, true_cnt, cnt_mul);
    
    vector<double> inv_cnt(getVecSize(group), cnt_mul);
    Valuation inputs_avg{{"inverse_cnt", inv_cnt}};
    encodeOrgInputforCmp(vec_input1, "lhs_avg", inputs_avg);
    encodeOrgInputforCmp(vec_input2, "rhs_avg", inputs_avg);
    encodeOrgInputFFT(vec_org, "fft_real", "fft_imag", inputs_avg);
    encryptInput(dag_for_gen_keys, inputs_avg,true);
    std::string str_input;
    savaInputTostr(dag_for_gen_keys, str_input);

    // Step 4: Server executes ciphertext averaging
    loadInputFromStr(dag_avg, str_input, true);  // Replace the previous round of input
    exeDag(dag_avg);
    std::string str_output_avg;
    savaOutputTostr(dag_avg, str_output_avg);

    // Step 5: Client decrypts
    loadOutputFromStr(dag_for_gen_keys, str_output_avg);

    std::vector<double> vec_results;
    getFFTDoubleOutputs(dag_for_gen_keys, MAX_CMP_NUM, "fft_out_real", "fft_out_imag",
                  vec_results);
    for (const auto& out_item : vec_results) {
      printf("\n avg %lf, true_sum %d,true_avg %lf \n\n", out_item, true_sum,
             true_sum / (double)true_cnt);
      break;
    }
  }

  releaseDag(group);
  releaseDag(dag_cnt_sum);
  releaseDag(dag_avg);
  releaseDag(dag_for_gen_keys);

}