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

// Comparison operator testing
#include "test_comm.h"

using namespace std;
using namespace iyfc;

namespace iyfctest {
//==
//<
//<=
//>
//>=

#define TEST_CMP_EXPR(NAME, EXPR_OP)                        \
  TEST(TEST_CMP, NAME) {                                    \
    DagPtr dag = initDag("CMP");                            \
    Expr lhs = setInputName(dag, "input_1");                \
    Expr rhs = setInputName(dag, "input_2");                \
    setOutput(dag, "cmp_out", lhs EXPR_OP rhs);             \
    compileDag(dag);                                        \
    genKeys(dag);                                           \
    vector<uint32_t> vec_input1;                            \
    vector<uint32_t> vec_input2;                            \
    vector<uint32_t> vec_plain_result(MAX_CMP_NUM, 0);      \
    for (int i = 0; i < MAX_CMP_NUM; i++) {                 \
      uint32_t value1 = rand() % MAX_CMP_NUM;               \
      uint32_t value2 = rand() % MAX_CMP_NUM;               \
      vec_input1.emplace_back(value1);                      \
      vec_input2.emplace_back(value2);                      \
      if (value1 EXPR_OP value2) vec_plain_result[i] = 1;   \
    }                                                       \
    Valuation inputs_1;                                     \
    encodeOrgInputforCmp(vec_input1, "input_1", inputs_1);  \
    encryptInput(dag, inputs_1);                            \
    Valuation inputs_2;                                     \
    encodeOrgInputforCmp(vec_input2, "input_2", inputs_2);  \
    encryptInput(dag, inputs_2);                            \
    exeDag(dag);                                            \
    std::vector<uint32_t> vec_result;                       \
    getCmpOutputs(dag, MAX_CMP_NUM, "cmp_out", vec_result); \
    for (int i = 0; i < MAX_CMP_NUM; i++) {                 \
      EXPECT_EQ(vec_result[i], vec_plain_result[i]);        \
    }                                                       \
    releaseDag(dag);                                        \
  }

TEST_CMP_EXPR(eq, ==);
TEST_CMP_EXPR(not_eq, !=);
TEST_CMP_EXPR(less, <);
TEST_CMP_EXPR(less_eq, <=);
TEST_CMP_EXPR(more, >);
TEST_CMP_EXPR(more_eq, >=);

}  // namespace iyfctest