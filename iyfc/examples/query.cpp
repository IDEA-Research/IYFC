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
 * @brief Example demonstrating ciphertext-plaintext comparison and index retrieval.
 */
void example_query_index() {
  DagPtr dag = initDag("query_index");
  int32_t total_num = 5;
  setCmpNumSize(dag, total_num);
  Expr lhs = setInputName(dag, "lhs");
  // Comparison condition 2
  Expr lhs_2 = setInputName(dag, "lhs_2");
  setOutput(dag, "cmp_index", (lhs <= 100) && (lhs_2 != 10));
  // step 3
  compileDag(dag);
  genKeys(dag);
  Valuation inputs;
  vector<uint32_t> vec_input1{1, 2, 5, 100, 1000};
  encodeOrgInputforCmp(vec_input1, "lhs", inputs);
  vector<uint32_t> vec_input2{4, 7, 10, 5, 100};
  encodeOrgInputforCmp(vec_input2, "lhs_2", inputs);
  encryptInput(dag, inputs);
  exeDag(dag);
  std::vector<uint32_t> vec_results;
  getCmpOutputs(dag, total_num, "cmp_index", vec_results);
  cout << "\n ------index result :---------- \n";
  print_vector(vec_results);
  releaseDag(dag);
}

/**
 * @brief Example demonstrating ciphertext-plaintext comparison and count retrieval.
 */
void example_query_cnt() {
  DagPtr dag = initDag("query_cnt");
  int32_t total_num = 5;
  setCmpNumSize(dag, total_num);
  Expr lhs = setInputName(dag, "lhs");
  // Comparison condition 2
  Expr lhs_2 = setInputName(dag, "lhs_2");
  setOutput(dag, "cmp_cnt", QueryCnt((lhs <= 100) && (lhs_2 != 10)));
  // step 3
  compileDag(dag);
  genKeys(dag);
  Valuation inputs;
  vector<uint32_t> vec_input1{1, 2, 5, 100, 1000};
  encodeOrgInputforCmp(vec_input1, "lhs", inputs);
  vector<uint32_t> vec_input2{4, 7, 10, 5, 100};
  encodeOrgInputforCmp(vec_input2, "lhs_2", inputs);
  encryptInput(dag, inputs);
  exeDag(dag);
  std::vector<uint32_t> vec_results;
  uint32_t ul_result;
  getCntOutput(dag, "cmp_cnt", ul_result);
  cout << "\n ------cnt result :---------- \n" << ul_result << endl;
  releaseDag(dag);
}

/**
 * @brief Example demonstrating ciphertext-plaintext comparison and sum retrieval.
 */
void example_query_sum() {
  DagPtr dag = initDag("query_sum");
  // Total number: 5 (if not set, defaults to MAX_CMP_NUM 1024)
  int32_t total_num = 5;
  setCmpNumSize(dag, total_num);
  Expr lhs = setInputName(dag, "lhs");
  Expr lhs_2 = setInputName(dag, "lhs_2");
  Expr fft_real = setInputName(dag, "fft_real");
  Expr fft_imag = setInputName(dag, "fft_imag");
  setOutput(dag, "sum_real", QuerySum(fft_real, (lhs <= 100) && (lhs_2 != 10)));
  setOutput(dag, "sum_imag", QuerySum(fft_imag, (lhs <= 100) && (lhs_2 != 10)));
  // step 3
  compileDag(dag);
  genKeys(dag);
  Valuation inputs;
  vector<uint32_t> vec_input1{1, 2, 5, 100, 1000};
  encodeOrgInputforCmp(vec_input1, "lhs", inputs);
  vector<uint32_t> vec_input2{4, 7, 9, 5, 100};
  encodeOrgInputforCmp(vec_input2, "lhs_2", inputs);
  // Summation for lhs
  encodeOrgInputFFT(vec_input1, "fft_real", "fft_imag", inputs);

  encryptInput(dag, inputs);
  exeDag(dag);
  std::vector<uint32_t> vec_results;
  getFFTOutputs(dag, total_num, "sum_real", "sum_imag", vec_results);

  cout << "\n ------sum result :---------- \n" << vec_results[0]<<endl;
  releaseDag(dag);
}

/*
    //input: server input id_s;
    //input: client input id_vec_c, data_vec(the same vectori size as id_vec_c);
    //output: rlt_vec in server party.

    step1: server generates fhe kesy:pk,sk,rlk,gal_key;
    step2: server encrypts id_s to cipher_id_s;
    step3: server sends cipher_id_s and pk/rlk/gal_key to client;
    step4: client encrypts id_vec_c to cipher_id_vec_c and data_vec to
   cipher_data_vec; step5: client evaluates ciphertext as the following: a) n =
   id_vec_c.size(); b) for 1 to n,  cipher_rlt_vec[i] = equal(cipher_id_s,
   cipher_id_vec_c[i])*cipher_data_vec[i]; step6: client send    cipher_rlt_vec
   to server step7: server decrypts cipher_rlt_vec to rlt_vec by sk.

    */

/**
 * @brief Example demonstrating a secure multi-party computation (SMPC) query.
 */
void example_query() {
  timespec begin_time = gettime();
  // Step 1: Build the comparison computation DAG (no serialization here)
  DagPtr dag = initDag("hello");
  uint32_t total_num = 5;
  setCmpNumSize(dag, total_num);

  // Step 2.1: Set input (supports multiple inputs)
  Expr lhs = setInputName(dag, "lhs");
  Expr rhs = setInputName(dag, "rhs");
  // Comparison condition 2
  Expr lhs_2 = setInputName(dag, "lhs_2");
  Expr rhs_2 = setInputName(dag, "rhs_2");
  // Output dimension data
  Expr fft_real = setInputName(dag, "fft_real");
  Expr fft_imag = setInputName(dag, "fft_imag");

  // Step 2.3: Set output (supports multiple outputs)

  setOutput(dag, "fft_out_real",
            QuerySum(fft_real, (lhs <= rhs) && (lhs_2 >= rhs_2)));
  setOutput(dag, "fft_out_imag",
            QuerySum(fft_imag, (lhs <= rhs) && (lhs_2 >= rhs_2)));

  // Step 3: Compile the DAG
  compileDag(dag);
  timespec compile_time = gettime();

  cout << "\n---TIMECOST COMPILE " << time_diff(begin_time, compile_time)
       << " ms-----\n";

  // Serialize DAG containing only signature information for decryption party use
  setDagSerializePara(dag, false, false, true, false, false, false);
  std::string str_only_sig;
  saveDagToStr(dag, str_only_sig);

  timespec sig_time = gettime();

  cout << "\n---TIMECOST saveSig " << time_diff(compile_time, sig_time)
       << " ms-----\n";

  // Serialize DAG containing genkey + signature information for key generation
  setDagSerializePara(dag, false, true, true, false, false, false);
  std::string str_genkeys_info;
  saveDagToStr(dag, str_genkeys_info);

  timespec save_sig_gen_time = gettime();

  cout << "\n---TIMECOST save_sig_gen_time "
       << time_diff(sig_time, save_sig_gen_time) << " ms-----\n";

  DagPtr dag_for_gen_keys = loadDagFromStr(str_genkeys_info);

  // Generate keys; at this point, the DAG contains the keys
  // Step 4
  genKeys(dag_for_gen_keys);

  timespec genkey_time = gettime();

  cout << "\n---TIMECOST genkey " << time_diff(save_sig_gen_time, genkey_time)
       << " ms-----\n";

  // Serialize exe_ctx only
  setDagSerializePara(dag_for_gen_keys, false, false, false, true, false,
                      false);

  std::string str_only_for_exe;

  saveKeysInfoToStr(dag_for_gen_keys, str_only_for_exe);

  timespec save_publickey_time = gettime();

  cout << "\n---TIMECOST save_publickey_time "
       << time_diff(genkey_time, save_publickey_time) << " ms-----\n";

  // Serialize decrpt_ctx only
  std::string str_only_for_decrpt;
  setDagSerializePara(dag_for_gen_keys, false, false, false, false, false,
                      true);
  saveKeysInfoToStr(dag_for_gen_keys, str_only_for_decrpt);
  std::string str_boot;
  if (checkIsBootstrapping(dag_for_gen_keys)) {
    std::stringstream stream;
    serializeByType(dag_for_gen_keys, BOOTSTRAPPING_KEY, stream);
    str_boot = stream.str();
  }

  timespec save_secretkey_time = gettime();

  cout << "\n---TIMECOST save_genkey_time "
       << time_diff(save_publickey_time, save_secretkey_time) << " ms-----\n";

  // Load keys - can be reused
  loadKeysFromStr(dag, str_only_for_exe);
  if (checkIsBootstrapping(dag)) {
    std::stringstream stream;
    stream.str(str_boot);
    deserializeByType(dag, BOOTSTRAPPING_KEY, stream);
    printf("success de boot_key\n");
  }

  timespec load_publickey_time = gettime();

  cout << "\n---TIMECOST load_publickey_time "
       << time_diff(save_secretkey_time, load_publickey_time) << " ms-----\n";

  // Decryption party
  DagPtr dag_can_decrypt = loadDagFromStr(str_only_sig);
  loadKeysFromStr(dag_can_decrypt, str_only_for_decrpt);

  timespec load_secretkey_time = gettime();

  cout << "\n---TIMECOST load_secretkey_time "
       << time_diff(load_publickey_time, load_secretkey_time) << " ms-----\n";

  int cnt = 1;
  while (cnt--) {
    timespec input_time = gettime();
    // DAG with public information can encrypt
    // Set and encrypt input vector<int64_t>
    Valuation inputs;
    vector<uint32_t> vec_input1{1, 2, 5, 100, 1000};
    vector<uint32_t> vec_input2{2, 2, 4, 100, 1001};
    vec_input1.resize(total_num);
    vec_input2.resize(total_num);
    encodeOrgInputforCmp(vec_input1, "lhs", inputs);
    encodeOrgInputforCmp(vec_input2, "rhs", inputs);

    vector<uint32_t> vec_input3{4, 7, 9, 5, 100};
    vector<uint32_t> vec_input4{4, 3, 5, 100, 100};
    encodeOrgInputforCmp(vec_input3, "lhs_2", inputs);
    encodeOrgInputforCmp(vec_input4, "rhs_2", inputs);

    // Process original data FFT
    vector<uint32_t> vec_org{918, 45, 2, 100, 1};
    int num_cnt = vec_org.size();
    encodeOrgInputFFT(vec_org, "fft_real", "fft_imag", inputs);

    // Encrypt
    encryptInput(dag_for_gen_keys, inputs);

    timespec encrept_time = gettime();

    cout << "\n---TIMECOST encryptInput " << time_diff(input_time, encrept_time)
         << " ms-----\n";

    // Serialize input
    std::string str_input;
    savaInputTostr(dag_for_gen_keys, str_input);

    timespec save_input_time = gettime();
    cout << "\n---TIMECOST savaInputTostr "
         << time_diff(encrept_time, save_input_time) << " ms-----\n";

    // DAG with node + public information + str_input can execute
    loadInputFromStr(dag, str_input);

    timespec load_input_time = gettime();
    cout << "\n---TIMECOST loadInputFromStr "
         << time_diff(save_input_time, load_input_time) << " ms-----\n";

    exeDag(dag);

    timespec exe_time = gettime();
    cout << "\n---TIMECOST exeDag " << time_diff(load_input_time, exe_time)
         << " ms-----\n";

    // Serialize output
    std::string str_output;
    savaOutputTostr(dag, str_output);

    // The final decodable information: sig + secretkey + output
    loadOutputFromStr(dag_can_decrypt, str_output);
    std::vector<uint32_t> vec_results;
    getFFTOutputs(dag_can_decrypt, num_cnt, "fft_out_real", "fft_out_imag",
                  vec_results);
    timespec de_time = gettime();
    cout << "\n---TIMECOST getFFTOutputs " << time_diff(exe_time, de_time)
         << " ms-----\n";

    cout << "\nresult : \n";
    print_vector(vec_results);
  }

  releaseDag(dag);
  releaseDag(dag_for_gen_keys);
  releaseDag(dag_can_decrypt);
}