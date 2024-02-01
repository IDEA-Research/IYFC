/*
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

#include "iyfc_include.h"
#include <fftw3.h>
#include "comm_include.h"
#include "dag/expr.h"
#include "dag/iyfc_dag.h"
#include "err_code.h"
#include "proto/save_load.h"
#include "util/clean_util.h"
#include "util/math_util.h"

using namespace std;
namespace iyfc {

DagPtr IYFC_SO_EXPORT initDag(const std::string& str_name, uint32_t u_size) {
  DagPtr dag_ptr(new Dag(str_name, u_size));
  return std::move(dag_ptr);
}

DagPtr IYFC_SO_EXPORT initDagGroup(const std::string& str_name,
                                   uint32_t u_size) {
  DagPtr dag_ptr(new DagGroup(str_name, u_size));
  return std::move(dag_ptr);
}

void IYFC_SO_EXPORT addDag(DagPtr group, DagPtr child) {
  static_cast<DagGroup*>(group)->addDag(child->getName(), child);
}

int IYFC_SO_EXPORT releaseDag(DagPtr dag_ptr) {
  clearDagNode(*dag_ptr);
  delete dag_ptr;
  return 0;
}

Expr IYFC_SO_EXPORT setInputName(DagPtr dag_ptr, const std::string& name) {
  return dag_ptr->setInput(name);
}

int IYFC_SO_EXPORT setOutput(DagPtr dag_ptr, const string& name,
                             const Expr& epxr) {
  dag_ptr->setOutput(name, epxr);
  return 0;
}

int IYFC_SO_EXPORT compileDag(DagPtr dag_ptr) {
  // LOG(LOGLEVEL::Debug, "\n before compileDag %s \n",
  // dag_ptr->toDOT().c_str());
  dag_ptr->doTranspile();
  // LOG(LOGLEVEL::Debug, "\n after compileDag %s \n",
  // dag_ptr->toDOT().c_str());
  return 0;
}

int IYFC_SO_EXPORT genKeys(DagPtr dag_ptr) {
  dag_ptr->genKey();
  return 0;
}

int IYFC_SO_EXPORT encryptInput(DagPtr dag_ptr, const Valuation& inputs,
                               bool replace) {
  for (const auto& item : inputs) {
    if (std::holds_alternative<std::vector<double>>(item.second)) {
      // vector
      const auto& v = std::get<std::vector<double>>(item.second);
      if (v.size() != dag_ptr->getVecSize()) {
        throw std::logic_error("input size not match ");
      }
    } else if (std::holds_alternative<std::vector<int64_t>>(item.second)) {
      // vector
      const auto& v = std::get<std::vector<int64_t>>(item.second);
      if (v.size() != dag_ptr->getVecSize()) {
        throw std::logic_error("input size not match ");
      }
    } else if (std::holds_alternative<double>(item.second)) {
      LOG(LOGLEVEL::Trace, "input one double");
    } else if (std::holds_alternative<int64_t>(item.second)) {
      LOG(LOGLEVEL::Trace, "input one int64_t");
    }
  }
  dag_ptr->encryptInput(inputs, replace);
  return 0;
}

// exedag

int IYFC_SO_EXPORT exeDag(DagPtr dag_ptr,
                          const std::unordered_set<std::string>& set_inputs) {
  dag_ptr->executor();
  return 0;
}

// decrpt output
int IYFC_SO_EXPORT decryptOutput(DagPtr dag_ptr, Valuation& outputs) {
  dag_ptr->getDecryptOutput(outputs);
  return 0;
}

//
int IYFC_SO_EXPORT setDagSerializePara(DagPtr dag_ptr, bool need_node_info,
                                       bool need_genkey_info,
                                       bool need_sig_info, bool need_exe_ctx,
                                       bool need_encrpt_ctx,
                                       bool need_decrypt_ctx) {
  dag_ptr->m_serialize_para = std::make_unique<DagSerializePara>(
      need_node_info, need_genkey_info, need_sig_info, need_exe_ctx,
      need_encrpt_ctx, need_decrypt_ctx);
  return 0;
}
int IYFC_SO_EXPORT saveDagToStr(DagPtr dag_ptr, std::string& str_dag) {
  str_dag = saveToString(*dag_ptr);
  if (str_dag.empty()) {
    throw std::logic_error("save dag to string err");
  }
  return 0;
}
DagPtr IYFC_SO_EXPORT loadDagFromStr(const std::string& str_dag) {
  unique_ptr<Dag> dag = loadFromString<unique_ptr<Dag>>(str_dag);
  if (dag != nullptr) {
    DagPtr dag_ptr(std::move(dag.release()));
    dag.reset();
    return std::move(dag_ptr);
  } else {
    throw std::logic_error("loadDagFromStr Err");
  }
}

int IYFC_SO_EXPORT saveGroupNodesToStr(DagPtr dag_ptr, std::string& str_dag) {
  // Turn down
  DagGroup* group = dynamic_cast<DagGroup*>(dag_ptr);
  str_dag = saveToString(*group);
  if (str_dag.empty()) {
    throw std::logic_error("save dag to string err");
  }
  return 0;
}
DagPtr IYFC_SO_EXPORT loadGroupNodesFromStr(const std::string& str_dag) {
  unique_ptr<DagGroup> group = loadFromString<unique_ptr<DagGroup>>(str_dag);
  if (group != nullptr) {
    DagPtr dag_ptr(std::move(dynamic_cast<Dag*>(group.release())));
    group.reset();
    return std::move(dag_ptr);
  } else {
    throw std::logic_error("loadDagFromStr Err");
  }
}

DagPtr IYFC_SO_EXPORT getChildDagByName(DagPtr dag_ptr, const std::string& name){
   DagGroup* group = dynamic_cast<DagGroup*>(dag_ptr);
   return group->getChildDagByName(name);
}

int IYFC_SO_EXPORT saveKeysInfoToStr(DagPtr dag_ptr, std::string& str_keys) {
  THROW_ON_ERROR(dag_ptr->saveAloInfoToStr(str_keys), "saveKeysInfoToStr");
  return 0;
}
int IYFC_SO_EXPORT loadKeysFromStr(DagPtr dag_ptr,
                                   const std::string& str_keys) {
  dag_ptr->loadAloInfoFromStr(str_keys);
  return 0;
}

int IYFC_SO_EXPORT savaInputTostr(DagPtr dag_ptr, std::string& str_input) {
  dag_ptr->saveInputToStr(str_input);
  return 0;
}
int IYFC_SO_EXPORT loadInputFromStr(DagPtr dag_ptr,
                                    const std::string& str_input,
                                    bool replace) {
  dag_ptr->loadInputFromStr(str_input, replace);
  return 0;
}

int IYFC_SO_EXPORT savaOutputTostr(DagPtr dag_ptr, std::string& str_output) {
  dag_ptr->saveOutputToStr(str_output);
  return 0;
}
int IYFC_SO_EXPORT loadOutputFromStr(DagPtr dag_ptr,
                                     const std::string& str_input) {
  dag_ptr->loadOutputFromStr(str_input);
  return 0;
}

int IYFC_SO_EXPORT encodeOrgInputforCmp(const std::vector<uint32_t>& vec_org,
                                        const string& input_name,
                                        Valuation& inputs) {
  uint32_t num_cnt = vec_org.size();

  if (num_cnt > MAX_CMP_NUM) {
    throw std::logic_error("max cmp  once");
    return CMP_NUM_LIMIT;
  }
  uint32_t p = CMP_P;
  uint32_t composemod = (p - 1) / 2 + 1;
  // The polynomial dimension is currently fixed at 16384

  vector<double> compose_vec_x;
  uint32_t bits = CMP_BIT_LEN;

  for (const auto& item : vec_org) {
    vector<uint32_t> tmp_vec = decimalConvert(item, composemod, bits);
    compose_vec_x.insert(compose_vec_x.end(), tmp_vec.begin(), tmp_vec.end());
  }

  compose_vec_x.resize(CMP_DAG_SIZE);
  inputs[input_name] = compose_vec_x;
  return 0;
}

int IYFC_SO_EXPORT getCmpOutputs(DagPtr dag_ptr, uint32_t num_cnt,
                                 const std::string& result_name,
                                 std::vector<uint32_t>& vec_results) {
  uint32_t p = CMP_P;           // Fixed decomposition base of 3
  uint32_t bits = CMP_BIT_LEN;  // Fixed to 32 bits
  Valuation outputs;
  dag_ptr->getDecryptOutput(outputs);

  if (outputs.find(result_name) != outputs.end()) {
    const auto& v = std::get<std::vector<double>>(outputs[result_name]);
    uint32_t len = bits * num_cnt;
    for (uint32_t i = 0; i < len; i++) {
      // Take the first bit of the result
      if (i % CMP_BIT_LEN == 0) {
        // Take the result modulo p   get the final matrix %p
        uint32_t tmp_one = uint32_t(round(v[i])) % p;
        // =1 indicates x < y, =0 (3) indicates x >= y
        vec_results.emplace_back(tmp_one);
      }
    }

  } else {
    throw std::logic_error("err outputs");
    return CMP_ERR_OUTPUT;
  }
  return 0;
}

// x encodes the entire array to be sorted as [1,2,3,4] -> x =
// [1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4]
int IYFC_SO_EXPORT encodeSortUpInput(const vector<uint32_t>& vec_input,
                                     vector<uint32_t>& vec_x) {
  uint32_t input_size = vec_input.size();
  for (const auto& item : vec_input) {
    vector<uint32_t> tmp_vec(input_size, item);
    vec_x.insert(vec_x.end(), tmp_vec.begin(), tmp_vec.end());
  }
  return 0;
}
// y encodes the array to be sorted by repeating each element, e.g., [1,2,3,4] -> y =
// [1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4]
int IYFC_SO_EXPORT encodeSortDownInput(const vector<uint32_t>& vec_input,
                                       vector<uint32_t>& vec_y) {
  uint32_t input_size = vec_input.size();
  while (input_size--) {
    vec_y.insert(vec_y.end(), vec_input.begin(), vec_input.end());
  }
  return 0;
}

int IYFC_SO_EXPORT encodeOrgInputforSort(const std::vector<uint32_t>& vec_org,
                                         Valuation& inputs) {
  if (vec_org.size() > 16) {
    throw std::logic_error("vec_org size must less than 16");
  }
  uint32_t p = CMP_P;
  uint32_t composemod = (p - 1) / 2 + 1;

  uint32_t d = 2, l_first = 2, l_second = 2, l_third = 2, l_fourth = 2;

  // Input after base decomposition
  vector<uint32_t> vec_x;
  vector<uint32_t> vec_y;
  encodeSortUpInput(vec_org, vec_x);
  encodeSortDownInput(vec_org, vec_y);

  // Data preprocessing
  vector<double> compose_vec_x;
  vector<double> compose_vec_y;
  uint32_t bits = CMP_BIT_LEN;
  for (const auto& item : vec_x) {
    vector<uint32_t> tmp_vec = decimalConvert(item, composemod, bits);
    compose_vec_x.insert(compose_vec_x.end(), tmp_vec.begin(), tmp_vec.end());
  }

  for (const auto& item : vec_y) {
    vector<uint32_t> tmp_vec = decimalConvert(item, composemod, bits);
    compose_vec_y.insert(compose_vec_y.end(), tmp_vec.begin(), tmp_vec.end());
  }

  compose_vec_x.resize(CMP_DAG_SIZE);
  compose_vec_y.resize(CMP_DAG_SIZE);
  inputs["x"] = compose_vec_x;
  inputs["y"] = compose_vec_y;
  return 0;
}

DagPtr IYFC_SO_EXPORT buildSortDag(const std::string& dag_name) {
  // The polynomial dimension is currently fixed at 16384
  DagPtr dag_ptr(new Dag(dag_name, CMP_DAG_SIZE));

  Expr input_expr_x = dag_ptr->setInput("x");
  Expr input_expr_y = dag_ptr->setInput("y");
  Expr input_expr_z = input_expr_x - input_expr_y;
  Expr lt_result;
  Expr eq_result;

  if (CMP_P == 3) {
    getCmpExprP3(input_expr_z, lt_result, eq_result);
  } else {
    getCmpExprP7(input_expr_z, lt_result, eq_result);
  }

  // Output
  dag_ptr->setOutput("result_out_less", lt_result);
  dag_ptr->setOutput("result_out_eq", eq_result);
  // DagPtr dag(dag_ptr.get());
  return std::move(dag_ptr);
}

int IYFC_SO_EXPORT
getSortOutputs(DagPtr dag_ptr, uint32_t num_cnt,
               std::vector<vector<uint32_t>>& matrix_result) {
  Valuation outputs;
  dag_ptr->getDecryptOutput(outputs);
  // Matrix result
  uint32_t bits = CMP_BIT_LEN;
  uint32_t p = CMP_P;

  if (outputs.find("result_out_less") != outputs.end()) {
    const auto& v = std::get<std::vector<double>>(outputs["result_out_less"]);
    uint32_t len = bits * num_cnt * num_cnt;
    uint32_t tmp_cout = 0;

    vector<uint32_t> vec_one;
    for (uint32_t i = 0; i < len; i++) {
      // Take the first bit of the result
      if (i % bits == 0) {
        // Take the result modulo p   get the final matrix %p
        uint32_t tmp_one = uint32_t(round(v[i])) % p;
        vec_one.emplace_back(tmp_one);
        tmp_cout++;
        // cout << v[i] << " ";
        if (tmp_cout == num_cnt) {
          tmp_cout = 0;
          matrix_result.emplace_back(std::move(vec_one));
        }
      }
    }

  } else {
    throw std::logic_error("getSortOutputs than 32");
  }
  return 0;
}

void IYFC_SO_EXPORT setScale(DagPtr dag_ptr, uint32_t u_scale) {
  dag_ptr->m_scale = u_scale;
}

// void IYFC_SO_EXPORT setOutputRange(DagPtr dag_ptr, uint32_t u_rangle) {
//   dag_ptr->configOutputRange(u_rangle);
// }

int IYFC_SO_EXPORT encodeOrgInputFFT(const std::vector<uint32_t>& vec_org,
                                     const std::string& input_name_real,
                                     const std::string& input_name_imag,
                                     Valuation& inputs) {
  std::vector<double> vec_real;
  std::vector<double> vec_imag;

  FastFourierTransform fft_helper(FFT_N, FFTW_FORWARD);

  for (const auto& item : vec_org) {
    std::vector<int> vec_num;
    getNumReVec(item, vec_num);
    for (int i = 0; i < FFT_N; i++) {
      fft_helper.m_in[i][0] = double(vec_num[i]);
      fft_helper.m_in[i][1] = 0;
    }
    // fft
    fft_helper.fft();
    for (int i = 0; i < FFT_N; i++) {
      vec_real.push_back(fft_helper.m_out[i][0]);
      vec_imag.push_back(fft_helper.m_out[i][1]);
    }
  }

  vec_real.resize(CMP_DAG_SIZE);
  vec_imag.resize(CMP_DAG_SIZE);
  inputs[input_name_real] = vec_real;
  inputs[input_name_imag] = vec_imag;
  return 0;
}

int IYFC_SO_EXPORT getCntOutput(DagPtr dag_ptr, const string& cnt_name,
                                uint32_t& ul_result) {
  Valuation outputs;
  dag_ptr->getDecryptOutput(outputs);

  // Process the real part
  std::vector<double> vec_cnt;
  if (outputs.find(cnt_name) != outputs.end()) {
    vec_cnt = std::get<std::vector<double>>(outputs[cnt_name]);
    ul_result = vec_cnt[0];
  }
  return 0;
}

int IYFC_SO_EXPORT getCntRandomOutput(DagPtr dag_ptr, const string& cnt_name,
                                      double& f_result) {
  Valuation outputs;
  dag_ptr->getDecryptOutput(outputs);

  // Process the real part
  std::vector<double> vec_cnt;
  if (outputs.find(cnt_name) != outputs.end()) {
    vec_cnt = std::get<std::vector<double>>(outputs[cnt_name]);
    f_result = vec_cnt[0];
  }
  return 0;
}

int IYFC_SO_EXPORT getFFTOutputs(DagPtr dag_ptr, uint32_t num_cnt,
                                 const string& output_real_name,
                                 const string& output_imag_name,
                                 std::vector<uint32_t>& vec_results) {
  std::vector<double> tmp_vec;
  getFFTDoubleOutputs(dag_ptr, num_cnt, output_real_name, output_imag_name,
                      tmp_vec);

  std::for_each(tmp_vec.begin(), tmp_vec.end(),
                [&](double tmp) { vec_results.emplace_back(std::round(tmp)); });

  return 0;
}

int IYFC_SO_EXPORT getFFTDoubleOutputs(DagPtr dag_ptr, uint32_t num_cnt,
                                       const string& output_real_name,
                                       const string& output_imag_name,
                                       std::vector<double>& vec_results) {
  Valuation outputs;
  dag_ptr->getDecryptOutput(outputs);

  // Process the real part
  std::vector<double> vec_real;
  if (outputs.find(output_real_name) != outputs.end()) {
    vec_real = std::get<std::vector<double>>(outputs[output_real_name]);

  } else {
    throw std::logic_error("err real outputs");
  }

  std::vector<double> vec_imag;
  if (outputs.find(output_imag_name) != outputs.end()) {
    vec_imag = std::get<std::vector<double>>(outputs[output_imag_name]);
  } else {
    throw std::logic_error("err real outputs");
  }
  int total_cnt = num_cnt * FFT_N;
  if (vec_real.size() < total_cnt || vec_imag.size() < total_cnt) {
    throw std::logic_error("err complex outputs size");
  }

  vec_real.resize(total_cnt);
  vec_imag.resize(total_cnt);

  // 32 bits represent one number
  // If the data is in the form [n,n,n....], performing ifft does not affect the final result - [n,0,0...]
  FastFourierTransform fft_helper(FFT_N, FFTW_BACKWARD);
  int cur_index = 0;
  for (int i = 0; i < total_cnt; i++) {
    fft_helper.m_in[cur_index][0] = vec_real[i];
    fft_helper.m_in[cur_index][1] = vec_imag[i];
    cur_index++;

    if (cur_index == FFT_N) {
      fft_helper.fft();
      vec_results.emplace_back(getComplexNum(fft_helper.m_out, FFT_N));
      cur_index = 0;
    }
  }
  return 0;
}

int IYFC_SO_EXPORT getFFTSumOutputs(DagPtr dag_ptr, uint32_t num_cnt,
                                    const std::string& output_real_name,
                                    const std::string& output_imag_name,
                                    uint32_t& sum_result) {
  std::vector<uint32_t> vec_results;
  THROW_ON_ERROR(getFFTOutputs(dag_ptr, num_cnt, output_real_name,
                                output_imag_name, vec_results),
                  "getFFTSumOutputs");

  if (vec_results.size() > 0) sum_result = vec_results[0];

  return 0;
}

bool IYFC_SO_EXPORT checkIsBootstrapping(DagPtr dag_ptr) {
  return dag_ptr->m_enable_bootstrap;
}

int IYFC_SO_EXPORT serializeByType(DagPtr dag_ptr, SERIALIZE_DATA_TYPE type,
                                   std::ostream& stream) {
  THROW_ON_ERROR(dag_ptr->saveByType(type, stream), "saveByType");
  return 0;
}

int IYFC_SO_EXPORT deserializeByType(DagPtr dag_ptr, SERIALIZE_DATA_TYPE type,
                                     std::istream& stream) {
  THROW_ON_ERROR(dag_ptr->loadByType(type, stream), "loadByType");
  return 0;
}

uint32_t IYFC_SO_EXPORT getVecSize(DagPtr dag_ptr) {
  return dag_ptr->getVecSize();
}

void IYFC_SO_EXPORT setCmpNumSize(DagPtr dag_ptr, uint32_t num_cnt) {
  dag_ptr->setNumSize(num_cnt);
}

std::vector<std::string> IYFC_SO_EXPORT getLibInfo(DagPtr dag_ptr) {
  return dag_ptr->getLibInfo();
}

uint64_t IYFC_SO_EXPORT getNextNodeIndex(DagPtr dag_ptr) {
  return dag_ptr->getNextNodeIndex();
}
void IYFC_SO_EXPORT setNextNodeIndex(DagPtr dag_ptr, uint64_t next_node_index) {
  dag_ptr->setNextNodeIndex(next_node_index);
}

std::string IYFC_SO_EXPORT toGraph(DagPtr dag_ptr) { return dag_ptr->toDOT(); }

}  // namespace iyfc