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

void example_serialization() {
  // Step 1: Build the DAG
  DagPtr dag = initDag("hello");
  // Input
  Expr input_expr = setInputName(dag, "x");
  Expr input_expr_y = setInputName(dag, "y");
  // Expr input_expr_y = setInputName(dag, "y");
  // Output
  setOutput(dag, "z", (input_expr >> 8096) * 0.0);
  // Step 3: Compile the DAG
  compileDag(dag);
  // Serialize DAG containing only signature information for decryption party use
  setDagSerializePara(dag, false, false, true, false, false, false);
  std::string str_only_sig;
  saveDagToStr(dag, str_only_sig);

  // Serialize DAG containing genkey + signature information for key generation by other programs
  setDagSerializePara(dag, false, true, true, false, false, false);
  std::string str_genkeys_info;
  saveDagToStr(dag, str_genkeys_info);

  DagPtr dag_for_gen_keys = loadDagFromStr(str_genkeys_info);

  // Generate keys; at this point, the DAG contains the keys
  // Step 4
  genKeys(dag_for_gen_keys);

  // Serialize exe_ctx only
  setDagSerializePara(dag_for_gen_keys, false, false, false, true, false,
                      false);
  std::string str_only_for_exe;
  saveKeysInfoToStr(dag_for_gen_keys, str_only_for_exe);
  // Serialize decrpt_ctx only
  std::string str_only_for_decrpt;
  setDagSerializePara(dag_for_gen_keys, false, false, false, false, false,
                      true);
  saveKeysInfoToStr(dag_for_gen_keys, str_only_for_decrpt);

  // Encrypt
  Valuation inputs{{"x", 10.0}, {"y", 10.0}};
  encryptInput(dag_for_gen_keys, inputs);

   // Serialize input
  std::string str_input;
  savaInputTostr(dag_for_gen_keys, str_input);
  // DAG with node + public information + str_input can execute
  loadKeysFromStr(dag, str_only_for_exe);

  loadInputFromStr(dag, str_input);

  exeDag(dag);

  // Serialize output
  std::string str_output;
  // err
  savaOutputTostr(dag, str_output);

  // The final decodable information: sig + secretkey + output
  DagPtr dag_can_decrypt = loadDagFromStr(str_only_sig);
  loadKeysFromStr(dag_can_decrypt, str_only_for_decrpt);

  // err !!!
  loadOutputFromStr(dag_can_decrypt, str_output);

  Valuation outputs;

  decryptOutput(dag_can_decrypt, outputs);
  printValuation(outputs);

  releaseDag(dag);
  releaseDag(dag_for_gen_keys);
  releaseDag(dag_can_decrypt);
}