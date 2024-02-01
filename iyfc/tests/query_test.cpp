
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

// Test query example
namespace iyfctest {
// ||
// &&
// querysum
// querycnt
#define TEST_QUERY_EXPR(NAME, CMP_EXPR)                             \
  TEST(TEST_QUERY, NAME) {                                          \
    DagPtr dag = initDag("QUERY");                                  \
    Expr lhs_1 = setInputName(dag, "lhs");                          \
    Expr rhs_1 = setInputName(dag, "rhs");                          \
    Expr lhs_2 = setInputName(dag, "lhs_2");                        \
    Expr rhs_2 = setInputName(dag, "rhs_2");                        \
    Expr fft_real = setInputName(dag, "fft_real");                  \
    Expr fft_imag = setInputName(dag, "fft_imag");                  \
    setOutput(dag, "fft_out_real", QueryRow(fft_real, CMP_EXPR));   \
    setOutput(dag, "fft_out_imag", QueryRow(fft_imag, CMP_EXPR));   \
    compileDag(dag);                                                \
    genKeys(dag);                                                   \
    Valuation inputs;                                               \
    vector<uint32_t> vec_input1;                                    \
    vector<uint32_t> vec_input2;                                    \
    vector<uint32_t> vec_input3;                                    \
    vector<uint32_t> vec_input4;                                    \
    vector<uint32_t> vec_org;                                       \
    vector<uint32_t> vec_plain_result(MAX_CMP_NUM, 0);              \
    for (int i = 0; i < MAX_CMP_NUM; i++) {                         \
      uint32_t lhs_1 = rand() % MAX_CMP_NUM;                        \
      uint32_t rhs_1 = rand() % MAX_CMP_NUM;                        \
      uint32_t lhs_2 = rand() % MAX_CMP_NUM;                        \
      uint32_t rhs_2 = rand() % MAX_CMP_NUM;                        \
      uint32_t org = rand() % 10240;                                \
      vec_input1.emplace_back(lhs_1);                               \
      vec_input2.emplace_back(rhs_1);                               \
      vec_input3.emplace_back(lhs_2);                               \
      vec_input4.emplace_back(rhs_2);                               \
      vec_org.emplace_back(org);                                    \
      if (CMP_EXPR) vec_plain_result[i] = org;                      \
    }                                                               \
    encodeOrgInputforCmp(vec_input1, "lhs", inputs);                \
    encodeOrgInputforCmp(vec_input2, "rhs", inputs);                \
    encodeOrgInputforCmp(vec_input3, "lhs_2", inputs);              \
    encodeOrgInputforCmp(vec_input4, "rhs_2", inputs);              \
    encodeOrgInputFFT(vec_org, "fft_real", "fft_imag", inputs);     \
    encryptInput(dag, inputs);                                      \
    exeDag(dag);                                                    \
    std::vector<uint32_t> vec_result;                               \
    getFFTOutputs(dag, MAX_CMP_NUM, "fft_out_real", "fft_out_imag", \
                  vec_result);                                      \
    for (int i = 0; i < MAX_CMP_NUM; i++) {                         \
      EXPECT_EQ(vec_result[i], vec_plain_result[i]);                \
    }                                                               \
  }

TEST_QUERY_EXPR(query_eq_one, lhs_1 == rhs_1);
TEST_QUERY_EXPR(query_not_eq_one, lhs_1 != lhs_2);
TEST_QUERY_EXPR(query_less_one, lhs_1 < rhs_1);
TEST_QUERY_EXPR(query_less_eq_one, lhs_1 <= rhs_1);
TEST_QUERY_EXPR(query_more_one, lhs_1 > rhs_1);
TEST_QUERY_EXPR(query_more_eq_one, lhs_1 >= rhs_1);

TEST_QUERY_EXPR(query_eq_and_two, (lhs_1 == rhs_1) && (lhs_2 == rhs_2));
TEST_QUERY_EXPR(query_eq_or_two, (lhs_1 == rhs_1) || (lhs_2 == rhs_2));
TEST_QUERY_EXPR(query_less_and_two, (lhs_1 < rhs_1) && (lhs_2 != rhs_2));
TEST_QUERY_EXPR(query_less_or_two, (lhs_1 <= rhs_1) || (lhs_2 != rhs_2));
TEST_QUERY_EXPR(query_more_and_two, (lhs_1 > rhs_1) && (lhs_2 > rhs_2));
TEST_QUERY_EXPR(query_more_or_two, (lhs_1 >= rhs_1) || (lhs_2 > rhs_2));

uint32_t plain_num_1 = rand() % MAX_CMP_NUM;
uint32_t plain_num_2 = rand() % MAX_CMP_NUM;

TEST_QUERY_EXPR(query_eq_plain, lhs_1 == plain_num_1);
TEST_QUERY_EXPR(query_not_eq_plain, lhs_1 != plain_num_1);
TEST_QUERY_EXPR(query_less_plain, lhs_1 < plain_num_1);
TEST_QUERY_EXPR(query_less_eq_plain, lhs_1 <= plain_num_1);
TEST_QUERY_EXPR(query_more_plain, lhs_1 > plain_num_1);
TEST_QUERY_EXPR(query_more_eq_plain, lhs_1 >= plain_num_1);

TEST_QUERY_EXPR(query_eq_and_plain,
                (lhs_1 == plain_num_1) && (lhs_2 == plain_num_2));
TEST_QUERY_EXPR(query_eq_or_plain,
                (lhs_1 == plain_num_1) || (lhs_2 == plain_num_2));
TEST_QUERY_EXPR(query_less_and_plain,
                (lhs_1 < plain_num_1) && (lhs_2 != plain_num_2));
TEST_QUERY_EXPR(query_less_or_plain,
                (lhs_1 <= plain_num_1) || (lhs_2 != plain_num_2));
TEST_QUERY_EXPR(query_more_and_plain,
                (lhs_1 > plain_num_1) && (lhs_2 > plain_num_2));
TEST_QUERY_EXPR(query_more_or_plain,
                (lhs_1 >= plain_num_1) || (lhs_2 > plain_num_2));

#define TEST_QUERY_SUM(NAME, CMP_EXPR)                              \
  TEST(TEST_QUERY, NAME) {                                          \
    DagPtr dag = initDag("QUERY");                                  \
    Expr lhs_1 = setInputName(dag, "lhs");                          \
    Expr rhs_1 = setInputName(dag, "rhs");                          \
    Expr lhs_2 = setInputName(dag, "lhs_2");                        \
    Expr rhs_2 = setInputName(dag, "rhs_2");                        \
    Expr fft_real = setInputName(dag, "fft_real");                  \
    Expr fft_imag = setInputName(dag, "fft_imag");                  \
    setOutput(dag, "fft_out_real", QuerySum(fft_real, CMP_EXPR));   \
    setOutput(dag, "fft_out_imag", QuerySum(fft_imag, CMP_EXPR));   \
    compileDag(dag);                                                \
    genKeys(dag);                                                   \
    Valuation inputs;                                               \
    vector<uint32_t> vec_input1;                                    \
    vector<uint32_t> vec_input2;                                    \
    vector<uint32_t> vec_input3;                                    \
    vector<uint32_t> vec_input4;                                    \
    vector<uint32_t> vec_org;                                       \
    uint32_t plain_result = 0;                                      \
    for (int i = 0; i < MAX_CMP_NUM; i++) {                         \
      uint32_t lhs_1 = rand() % MAX_CMP_NUM;                        \
      uint32_t rhs_1 = rand() % MAX_CMP_NUM;                        \
      uint32_t lhs_2 = rand() % MAX_CMP_NUM;                        \
      uint32_t rhs_2 = rand() % MAX_CMP_NUM;                        \
      uint32_t org = rand() % 1024;                                \
      vec_input1.emplace_back(lhs_1);                               \
      vec_input2.emplace_back(rhs_1);                               \
      vec_input3.emplace_back(lhs_2);                               \
      vec_input4.emplace_back(rhs_2);                               \
      vec_org.emplace_back(org);                                    \
      if (CMP_EXPR) plain_result += org;                            \
    }                                                               \
    encodeOrgInputforCmp(vec_input1, "lhs", inputs);                \
    encodeOrgInputforCmp(vec_input2, "rhs", inputs);                \
    encodeOrgInputforCmp(vec_input3, "lhs_2", inputs);              \
    encodeOrgInputforCmp(vec_input4, "rhs_2", inputs);              \
    encodeOrgInputFFT(vec_org, "fft_real", "fft_imag", inputs);     \
    encryptInput(dag, inputs);                                      \
    exeDag(dag);                                                    \
    std::vector<uint32_t> vec_result;                               \
    getFFTOutputs(dag, MAX_CMP_NUM, "fft_out_real", "fft_out_imag", \
                  vec_result);                                      \
    EXPECT_EQ(vec_result[0], plain_result);                         \
  }

TEST_QUERY_SUM(query_more_and_two_sum, (lhs_1 > rhs_1) && (lhs_2 > rhs_2));
TEST_QUERY_SUM(query_more_or_two_sum, (lhs_1 >= rhs_1) || (lhs_2 > rhs_2));
TEST_QUERY_SUM(query_more_and_plain_sum,
                (lhs_1 > plain_num_1) && (lhs_2 > plain_num_2));
TEST_QUERY_SUM(query_more_or_plain_sum,
                (lhs_1 >= plain_num_1) || (lhs_2 > plain_num_2));

}  // namespace iyfctest