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

/*
serialization test
In addition, seal openfhe concrete library testing is also required

Contains: serialization deserialization
dag
genkey_info
sig
p_key
s_key
input
output

*/
namespace iyfctest {

void serFun(std::function<Expr(DagPtr dag)> expr_fun,
            std::function<void(Valuation& inputs, Valuation& out_puts_plain,
                               uint32_t vec_size)>
                input_fun,
            std::function<void(Valuation& outputs, Valuation& outputs_plain)>
                check_fun) {
  // Step 1: Build DAG
  DagPtr dag = initDag("hello");

  setOutput(dag, "z", expr_fun(dag));
  // Step 3: Compile DAG
  compileDag(dag);
  // Serialize DAG with only signature information for decryption
  setDagSerializePara(dag, false, false, true, false, false, false);
  std::string str_only_sig;
  saveDagToStr(dag, str_only_sig);

  // Serialize DAG with key generation and signature information
  setDagSerializePara(dag, false, true, true, false, false, false);
  std::string str_genkeys_info;
  saveDagToStr(dag, str_genkeys_info);

  DagPtr dag_for_gen_keys = loadDagFromStr(str_genkeys_info);

  // Step 4: Generate Keys
  genKeys(dag_for_gen_keys);

  // Serialize DAG with only execution context information
  setDagSerializePara(dag_for_gen_keys, false, false, false, true, false,
                      false);
  std::string str_only_for_exe;
  saveKeysInfoToStr(dag_for_gen_keys, str_only_for_exe);
  // Serialize DAG with only decryption context information
  std::string str_only_for_decrpt;
  setDagSerializePara(dag_for_gen_keys, false, false, false, false, false,
                      true);
  saveKeysInfoToStr(dag_for_gen_keys, str_only_for_decrpt);
  // Encrypt Inputs
  Valuation inputs;
  Valuation outputs_plain;
  input_fun(inputs, outputs_plain, getVecSize(dag));
  encryptInput(dag_for_gen_keys, inputs);
   // Serialize Inputs
  std::string str_input;
  savaInputTostr(dag_for_gen_keys, str_input);
  // Deserialize DAG with execution context and input information
  loadKeysFromStr(dag, str_only_for_exe);
  loadInputFromStr(dag, str_input);

  exeDag(dag);

  // Serialize Outputs
  std::string str_output;
  // err
  savaOutputTostr(dag, str_output);
  // can eventually be decrypted  sig  + scretkey + output
  DagPtr dag_can_decrypt = loadDagFromStr(str_only_sig);
  loadKeysFromStr(dag_can_decrypt, str_only_for_decrpt);
  // err !!!
  loadOutputFromStr(dag_can_decrypt, str_output);
  Valuation outputs;
  decryptOutput(dag_can_decrypt, outputs);

  check_fun(outputs, outputs_plain);

  releaseDag(dag);
  releaseDag(dag_for_gen_keys);
  releaseDag(dag_can_decrypt);
}

TEST(TEST_SERIALIZE, seal_ckks_ser_dag) {
  serFun(
      [&](DagPtr dag) -> Expr {
        Expr input_expr_x = setInputName(dag, "x");
        Expr input_expr_y = setInputName(dag, "y");
        return (input_expr_x + 10.0) * input_expr_y;
      },
      [&](Valuation& inputs, Valuation& out_puts_plain, uint32_t vec_size) {
        vector<double> vec_input_x;
        vector<double> vec_input_y;
        vector<double> vec_out;
        int64_t data_bound = (1 << 10);
        for (uint32_t i = 0; i < vec_size; i++) {
          double value1 = static_cast<double>(rand() % data_bound);
          double value2 = static_cast<double>(rand() % data_bound);
          vec_input_x.emplace_back(value1);
          vec_input_y.emplace_back(value2);
          vec_out.emplace_back((value1 + 10.0) * value2);
        }

        inputs["x"] = std::move(vec_input_x);
        inputs["y"] = std::move(vec_input_y);
        out_puts_plain["z"] = std::move(vec_out);
      },
      [&](Valuation& outputs, Valuation& outputs_plain) {
        check_result<double>(
            outputs, std::get<vector<double>>(outputs_plain["z"]), 0.001);
      });
}

TEST(TEST_SERIALIZE, seal_bfv_ser_dag) {
  serFun(
      [&](DagPtr dag) -> Expr {
        Expr input_expr_x = setInputName(dag, "x");
        Expr input_expr_y = setInputName(dag, "y");
        return (input_expr_x + 10) * input_expr_y;
      },
      [&](Valuation& inputs, Valuation& out_puts_plain, uint32_t vec_size) {
        vector<int64_t> vec_input_x;
        vector<int64_t> vec_input_y;
        vector<int64_t> vec_out;
        int64_t data_bound = (1 << 10);
        for (uint32_t i = 0; i < vec_size; i++) {
          int64_t value1 = static_cast<int64_t>(rand() % data_bound);
          int64_t value2 = static_cast<int64_t>(rand() % data_bound);
          vec_input_x.emplace_back(value1);
          vec_input_y.emplace_back(value2);
          vec_out.emplace_back((value1 + 10) * value2);
        }

        inputs["x"] = std::move(vec_input_x);
        inputs["y"] = std::move(vec_input_y);
        out_puts_plain["z"] = std::move(vec_out);
      },
      [&](Valuation& outputs, Valuation& outputs_plain) {
        check_result<int64_t>(
            outputs, std::get<vector<int64_t>>(outputs_plain["z"]), 0.001);
      });
}

TEST(TEST_SERIALIZE, openfhe_ckks_ser_dag) {
  serFun(
      [&](DagPtr dag) -> Expr {
        Expr x = setInputName(dag, "x");
        return ((((((((((((x * x - x) * x - x) * x - x) * x - x) * x - x) * x -
                      x) *
                         x -
                     x) *
                        x -
                    x) *
                       x -
                   x) *
                      x -
                  x) *
                     x -
                 x) *
                    x -
                x) *
                   x -
               x;
      },
      [&](Valuation& inputs, Valuation& out_puts_plain, uint32_t vec_size) {
        vector<double> vec_input_x;
        vector<double> vec_input_y;
        vector<double> vec_out;
        for (uint32_t i = 0; i < vec_size; i++) {
          double x = 2.0;
          vec_input_x.emplace_back(x);
          vec_out.emplace_back(
              ((((((((((((x * x - x) * x - x) * x - x) * x - x) * x - x) * x -
                     x) *
                        x -
                    x) *
                       x -
                   x) *
                      x -
                  x) *
                     x -
                 x) *
                    x -
                x) *
                   x -
               x) *
                  x -
              x);
        }

        inputs["x"] = std::move(vec_input_x);
        out_puts_plain["z"] = std::move(vec_out);
      },
      [&](Valuation& outputs, Valuation& outputs_plain) {
        check_result<double>(outputs,
                             std::get<vector<double>>(outputs_plain["z"]), 1);
      });
}

TEST(TEST_SERIALIZE, openfhe_bfv_ser_dag) {
  serFun(
      [&](DagPtr dag) -> Expr {
        Expr x = setInputName(dag, "x");
        return ((((((((((((x * x - x) * x - x) * x - x) * x - x) * x - x) * x -
                      x) *
                         x -
                     x) *
                        x -
                    x) *
                       x -
                   x) *
                      x -
                  x) *
                     x -
                 x) *
                    x -
                x) *
                   x -
               x + 1;
      },
      [&](Valuation& inputs, Valuation& out_puts_plain, uint32_t vec_size) {
        vector<int64_t> vec_input_x;
        vector<int64_t> vec_input_y;
        vector<int64_t> vec_out;
        for (uint32_t i = 0; i < vec_size; i++) {
          int64_t x = 1;
          vec_input_x.emplace_back(x);
          vec_out.emplace_back(
              ((((((((((((x * x - x) * x - x) * x - x) * x - x) * x - x) * x -
                     x) *
                        x -
                    x) *
                       x -
                   x) *
                      x -
                  x) *
                     x -
                 x) *
                    x -
                x) *
                   x -
               x) *
                  x -
              x + 1);
        }

        inputs["x"] = std::move(vec_input_x);
        out_puts_plain["z"] = std::move(vec_out);
      },
      [&](Valuation& outputs, Valuation& outputs_plain) {
        check_result<int64_t>(outputs,
                              std::get<vector<int64_t>>(outputs_plain["z"]), 1);
      });
}

/*concrete data size exceeds the protobuf limit*/

// TEST(TEST_SERIALIZE, concrete_ser_dag) {
//   serFun(
//       [&](DagPtr dag) -> Expr {
//         Expr input_expr_x = setInputName(dag, "x");
//         return (uint8_t)2 / input_expr_x;
//       },
//       [&](Valuation& inputs, Valuation& out_puts_plain, uint32_t vec_size) {
//         inputs["x"] = (uint8_t)2;
//         out_puts_plain["z"] = (uint8_t)(2 / 2);
//       },
//       [&](Valuation& outputs, Valuation& outputs_plain) {
//         EXPECT_EQ(std::get<uint8_t>(outputs["z"]),
//                   std::get<uint8_t>(outputs_plain["z"]));
//       });
// }

}  // namespace iyfctest