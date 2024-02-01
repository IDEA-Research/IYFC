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

#include "dag/iyfc_dag.h"
#include "test_comm.h"

using namespace std;
using namespace iyfc;
/*
1. Construct (super:)group, (children:)dag_cnt_sum and dag_avg
2. Verify accuracy after calculation
3. Serialization included
*/
namespace iyfctest {
TEST(GroupDagTest, TestGroupDag) {
#pragma region Dag Initiation
  // group
  DagPtr group = initDagGroup("group");

  // dag 1
  DagPtr dag_cnt_sum = initDag("chil_dag_cnt");
  Expr lhs = setInputName(dag_cnt_sum, "lhs");
  setOutput(dag_cnt_sum, "add_result", lhs * lhs * lhs + 1.0);
  addDag(group, dag_cnt_sum);

  // dag 2
  DagPtr dag_avg = initDag("child_dag_avg");
  setNextNodeIndex(dag_avg, getNextNodeIndex(group));
  Expr inverse_cnt = setInputName(dag_avg, "inverse_cnt");
  setOutput(dag_avg, "plus_result",
            1.0 * ((inverse_cnt >> 1)) + inverse_cnt * inverse_cnt);
  addDag(group, dag_avg);
#pragma endregion

#pragma region Process Data
  // group
  compileDag(group);
  setDagSerializePara(group, false, true, true, false, false,
                      false);  // configure how to serialize Dag objects
  std::string str_genkeys_info;
  saveDagToStr(group, str_genkeys_info);
  DagPtr dag_for_gen_keys = loadDagFromStr(str_genkeys_info);
  genKeys(dag_for_gen_keys);
  setDagSerializePara(dag_for_gen_keys, false, false, false, true, true, false);
  std::string str_only_for_exe;
  saveKeysInfoToStr(dag_for_gen_keys, str_only_for_exe);
  loadKeysFromStr(
      group,
      str_only_for_exe);  // group gets the public key. Deserialize the public
                          // key information from the string str_only_for_exe
                          // into the group object for ciphertext calculation.

  // dag 1
  vector<double> vec_input1;
  for (int i = 0; i < MAX_CMP_NUM; i++) {
    uint32_t lhs_1 = rand() % MAX_CMP_NUM;
    vec_input1.emplace_back(lhs_1);
  }
  Valuation inputs{{"lhs", vec_input1}};
  encryptInput(dag_cnt_sum, inputs);
  exeDag(dag_cnt_sum);
  std::string str_output_cnt;
  savaOutputTostr(dag_cnt_sum, str_output_cnt);
  loadOutputFromStr(dag_for_gen_keys, str_output_cnt);
  Valuation outputs;
  decryptOutput(dag_for_gen_keys, outputs);
  // predicted output
  vector<double> vec_out;
  for (auto& item : vec_input1) {
    vec_out.emplace_back(item * item * item + 1.0);
  }
  // verify output by assertion
  check_result<double>(outputs, vec_out, 0.001);

  // dag 2
  vector<double> vec_input2;
  vector<double> vec_input2_right;
  for (int i = 0; i < MAX_CMP_NUM; i++) {
    uint32_t lhs_2 = rand() % 2;
    vec_input2.emplace_back(lhs_2);
    if (i == (MAX_CMP_NUM - 1)) {
      vec_input2_right[0] = lhs_2;
    } else {
      vec_input2_right.emplace_back(lhs_2);
    }
  }
  Valuation inputs_avg{{"inverse_cnt", vec_input2}};
  encryptInput(dag_avg, inputs_avg, true);
  exeDag(dag_avg);
  std::string str_output_avg;
  savaOutputTostr(dag_avg, str_output_avg);
  loadOutputFromStr(dag_for_gen_keys, str_output_avg);
  Valuation outputs_plus;
  decryptOutput(dag_for_gen_keys, outputs_plus);
  // predicted output: vec_out_plus
  vector<double> vec_out_plus;
  for (int i = 0; i < MAX_CMP_NUM; i++) {
    vec_out_plus.emplace_back(1.0 * vec_input2_right[i] +
                              vec_input2[i] * vec_input2[i]);
  }
  // verify output by assertion
  check_result<double>(outputs_plus, vec_out_plus, 0.001);
#pragma endregion

  releaseDag(group);
  releaseDag(dag_avg);
  releaseDag(dag_cnt_sum);
  releaseDag(dag_for_gen_keys);
}

class GroupDagSerialTest : public ::testing::Test {
 protected:
  DagPtr group;
  DagPtr dag_cnt_sum;
  DagPtr dag_avg;
  vector<double> vec_out;
  vector<double> vec_out_plus;

  void SetUp() override {
#pragma region Dag Initiation
    // group
    group = initDagGroup("group");

    // dag 1
    dag_cnt_sum = initDag("child_dag_cnt");
    Expr lhs = setInputName(dag_cnt_sum, "lhs");
    setOutput(dag_cnt_sum, "add_result", lhs * lhs * lhs + 1.0);
    addDag(group, dag_cnt_sum);

    // dag 2
    dag_avg = initDag("child_dag_avg");
    setNextNodeIndex(dag_avg, getNextNodeIndex(group));
    Expr inverse_cnt = setInputName(dag_avg, "inverse_cnt");
    setOutput(dag_avg, "plus_result",
              1.0 * ((inverse_cnt >> 1)) + inverse_cnt * inverse_cnt);
    addDag(group, dag_avg);
#pragma endregion

#pragma region process data
    // group processing data
    compileDag(group);
    genKeys(group);

    // processing dag 1
    vector<double> vec_input1;
    for (int i = 0; i < MAX_CMP_NUM; i++) {
      uint32_t lhs_1 = rand() % MAX_CMP_NUM;
      vec_input1.emplace_back(lhs_1);
    }
    Valuation inputs{{"lhs", vec_input1}};
    encryptInput(dag_cnt_sum, inputs);
    exeDag(dag_cnt_sum);
    Valuation outputs;
    decryptOutput(dag_cnt_sum, outputs);
    // predicted output
    for (auto& item : vec_input1) {
      vec_out.emplace_back(item * item * item + 1.0);
    }

    // processing dag 2
    vector<double> vec_input2;
    vector<double> vec_input2_right;
    for (int i = 0; i < MAX_CMP_NUM; i++) {
      uint32_t lhs_2 = rand() % 2;
      vec_input2.emplace_back(lhs_2);
      if (i == (MAX_CMP_NUM - 1)) {
        vec_input2_right[0] = lhs_2;
      } else {
        vec_input2_right.emplace_back(lhs_2);
      }
    }
    Valuation inputs_avg{{"inverse_cnt", vec_input2}};
    encryptInput(dag_avg, inputs_avg, true);
    exeDag(dag_avg);
    Valuation outputs_plus;
    decryptOutput(dag_avg, outputs_plus);
    // predicted output: vec_out_plus
    for (int i = 0; i < MAX_CMP_NUM; i++) {
      vec_out_plus.emplace_back(1.0 * vec_input2_right[i] +
                                vec_input2[i] * vec_input2[i]);
    }

#pragma endregion
  }

  void TearDown() override {
    releaseDag(group);
    releaseDag(dag_avg);
    releaseDag(dag_cnt_sum);
  }
};

TEST_F(GroupDagSerialTest, SaveTest) {
  // serialize group to string
  setDagSerializePara(group, true, true, true, true, true, true);
  std::string str_dag_group;
  int result = saveGroupNodesToStr(group, str_dag_group);
  // check serialization succeed or not
  ASSERT_EQ(result, 0);
  ASSERT_FALSE(str_dag_group.empty());
}

TEST_F(GroupDagSerialTest, LoadTest) {
  // serialize group to string
  setDagSerializePara(group, true, true, true, true, true, true);
  std::string str_dag_group;
  saveGroupNodesToStr(group, str_dag_group);
  // load str to dag
  DagPtr loaded_dag_group = loadGroupNodesFromStr(str_dag_group);
  // check load successfully
  ASSERT_NE(loaded_dag_group, nullptr);
  EXPECT_EQ(loaded_dag_group->getName(), group->getName());
  releaseDag(loaded_dag_group);
}

// verify the number of subdag
TEST_F(GroupDagSerialTest, DagNumTest) {
  setDagSerializePara(group, true, true, true, true, true, true);
  std::string str_dag_group;
  saveGroupNodesToStr(group, str_dag_group);
  DagPtr loaded_dag_group = loadGroupNodesFromStr(str_dag_group);

  std::vector<iyfc::NodePtr> group_sinks = group->getSinks();
  std::vector<iyfc::NodePtr> loaded_sinks = loaded_dag_group->getSinks();

  EXPECT_EQ(group_sinks.size(), loaded_sinks.size());

  // verify if all subdags in group_sinks are in loaded_sinks
  for (const auto& group_sink : group_sinks) {
    bool found = false;
    for (const auto& loaded_sink : loaded_sinks) {
      if (group_sink->m_dag->getName() == loaded_sink->m_dag->getName()) {
        found = true;
        break;
      }
    }
    EXPECT_TRUE(found);
  }

  releaseDag(loaded_dag_group);
}

TEST_F(GroupDagSerialTest, TestOutput) {
  setDagSerializePara(group, true, true, true, true, true, true);
  std::string str_dag_group;
  saveGroupNodesToStr(group, str_dag_group);
  DagPtr loaded_dag_group = loadGroupNodesFromStr(str_dag_group);

  std::unordered_map<std::string, iyfc::NodePtr> dag_output_cnt =
      loaded_dag_group->getOutputs();
  std::unordered_map<std::string, iyfc::NodePtr> group_output_cnt =
      group->getOutputs();

  // for (const auto& item : dag_output_cnt){
  //   printf("%s ", item.first.c_str());
  //   cout << item.second;
  //   printf("//");
  // }

  // verify if all outputs setted in group are in loaded dag
  for (const auto& group_outp : group_output_cnt) {
    bool found = false;
    for (const auto& dag_outp : dag_output_cnt) {
      if (group_outp.first == dag_outp.first) {
        found = true;
        break;
      }
    }

    EXPECT_TRUE(found);
  }

  releaseDag(loaded_dag_group);
}

TEST_F(GroupDagSerialTest, ContinueCalculateTest) {
  // A saves the previous calculation state
  setDagSerializePara(group, true, true, true, true, true, true);
  std::string str_dag_group;
  saveGroupNodesToStr(group, str_dag_group);

  // B receives the calculation status represented by 'str_dag_group'
  DagPtr loaded_dag_group = loadGroupNodesFromStr(str_dag_group);
  DagPtr loaded_dag_cnt_sum =
      getChildDagByName(loaded_dag_group, "child_dag_cnt");
  DagPtr loaded_dag_avg = getChildDagByName(loaded_dag_group, "child_dag_avg");

  // B performs further calculation operations
  // 1. use 'child_dag_cnt'
  vector<double> vec_input1_b{1, 2, 5, 100, 1000};
  vec_input1_b.resize(getVecSize(loaded_dag_cnt_sum));
  Valuation inputs1_b{{"lhs", vec_input1_b}};
  encryptInput(loaded_dag_cnt_sum, inputs1_b);
  exeDag(loaded_dag_cnt_sum);
  Valuation outputs1_b;
  decryptOutput(loaded_dag_cnt_sum, outputs1_b);
  vector<double> vec_out1_b;
  for (auto& item : vec_input1_b) {
    vec_out1_b.emplace_back(item * item * item + 1.0);
  }
  vec_out1_b.resize(getVecSize(loaded_dag_cnt_sum));
  check_result<double>(outputs1_b, vec_out1_b, 0.001);

  // 2. use 'child_dag_avg'
  vector<double> vec_input2_b(getVecSize(loaded_dag_avg), 1);
  Valuation inputs2_b{{"inverse_cnt", vec_input2_b}};
  encryptInput(loaded_dag_avg, inputs2_b, true);
  exeDag(loaded_dag_avg);
  Valuation outputs2_b;
  decryptOutput(loaded_dag_avg, outputs2_b);
  vector<double> vec_out2_b(getVecSize(loaded_dag_avg), 2);
  check_result<double>(outputs2_b, vec_out2_b, 0.001);

  releaseDag(loaded_dag_group);
  releaseDag(loaded_dag_cnt_sum);
  releaseDag(loaded_dag_avg);
}

}  // namespace iyfctest