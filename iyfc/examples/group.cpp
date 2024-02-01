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
 * @brief Example demonstrating the use of a DagGoup.
 */
void example_group() {
  DagPtr group = initDagGroup("group");

  // DAG 1: Count and Sum
  DagPtr dag_cnt_sum = initDag("child_dag_cnt");
  Expr lhs = setInputName(dag_cnt_sum, "lhs");
  setOutput(dag_cnt_sum, "add_result", lhs*lhs*lhs + 1.0);
  addDag(group, dag_cnt_sum);

  // DAG 2: Average Calculation
  DagPtr dag_avg = initDag("child_dag_avg");
  setNextNodeIndex(dag_avg, getNextNodeIndex(group));
  Expr inverse_cnt = setInputName(dag_avg, "inverse_cnt");
  setOutput(dag_avg, "plus_result", 1.0 * ((inverse_cnt >> 1) + inverse_cnt*inverse_cnt));
  addDag(group, dag_avg);

  // Process data in the group
  compileDag(group);
  printf(" \n compileDag :\n");
  genKeys(group);

  setDagSerializePara(group, true, true, true, true, true, true);
  // std::string str_dag;
  // saveGroupNodesToStr(group, str_dag);
  // DagPtr new_group =loadGroupNodesFromStr(str_dag);
  // DagPtr new_child = getChildDagByName(new_group, "child_dag_cnt");
  // printf(" load group %s \n ",toGraph(new_group).c_str());
  // printf(" new_child %s \n ",toGraph(new_child).c_str());

  printf(" \n genkeys :\n");

  // Process data for DAG 1
  vector<double> vec_input1{1, 2, 5, 100, 1000};
  vec_input1.resize(getVecSize(group));
  Valuation inputs{{"lhs", vec_input1}};

  encryptInput(dag_cnt_sum, inputs);
  exeDag(dag_cnt_sum);
  Valuation outputs;
  decryptOutput(dag_cnt_sum, outputs);
  const auto& v = std::get<std::vector<double>>(outputs["add_result"]);
  printf(" \n add_result v size %zu:\n", v.size());
  for (int i = 0; i < 5; i++) {
    printf(" %f ,", v[i]);
  }

  // Process data for DAG 2
  printf("\nchild 2\n");
  vector<double> vec_input2(getVecSize(group),1);
  Valuation inputs_avg{{"inverse_cnt", vec_input2}};
  encryptInput(dag_avg, inputs_avg, true);
  printf(" encryptInput 2\n");
  exeDag(dag_avg);
  Valuation outputs_plus;
  printf(" decryptOutput 2\n");
  decryptOutput(dag_avg, outputs_plus);

  const auto& v_plus = std::get<std::vector<double>>(outputs_plus["plus_result"]);
  printf(" \n plus_result size %zu:\n", v_plus.size());
  for (int i = 0; i < 5; i++) {
    printf(" %f,", v_plus[i]);
  }


  releaseDag(group);
  releaseDag(dag_avg);
  releaseDag(dag_cnt_sum);
  

}