
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
#include "iyfc_include.h"
#include "test_comm.h"

using namespace std;
using namespace iyfc;

namespace iyfctest {
void checkLib(DagPtr dag, const string& lib_name) {
  vector<string> vec_libs = getLibInfo(dag);
  if (vec_libs.size() == 0) FAIL();
  EXPECT_EQ(vec_libs[0], lib_name);
}

#define TEST_EXPR_ONE_LIB(NAME, EXPR, TYPE, LIB_NAME)       \
  TEST(TEST_DECIDION, NAME) {                               \
    vector<TYPE> vec_input;                                 \
    int64_t data_bound = (2);                               \
    for (int i = 0; i < 1024; i++) {                        \
      TYPE value1 = static_cast<TYPE>(rand() % data_bound); \
      vec_input.emplace_back(value1);                       \
    }                                                       \
    DagPtr dag = initDag("DECISION", 1024);                 \
    Expr x = setInputName(dag, "x");                        \
    Expr z = EXPR;                                          \
    Valuation inputs{{"x", vec_input}};                     \
    Valuation output = execute(inputs, dag, z);             \
    checkLib(dag, #LIB_NAME);                               \
    releaseDag(dag);                                        \
  }

TEST_EXPR_ONE_LIB(use_seal_ckks, x* x * 1.0, double, seal_ckks)
// Boundary 11 layer multiplication
TEST_EXPR_ONE_LIB(
    use_seal_ckks_muldep11,
    ((((((((((x * x - x) * x - x) * x - x) * x - x) * x - x) * x - x) * x - x) *
           x -
       x) *
          x -
      x) *
         x -
     x) * x -
        x,
    double, seal_ckks);

// bfv
TEST_EXPR_ONE_LIB(use_seal_bfv, x* x + 1, int64_t, seal_bfv)

TEST_EXPR_ONE_LIB(
    use_seal_bfv_muldep11,
    ((((((((((x * x - x) * x - x) * x - x) * x - x) * x - x) * x - x) * x - x) *
           x -
       x) *
          x -
      x) *
         x -
     x) * x -
        1,
    int64_t, seal_bfv);

// Using openfhe cpu scenario>11
TEST_EXPR_ONE_LIB(
    use_openfhe_ckks_muldep11,
    ((((((((((((x * x - x) * x - x) * x - x) * x - x) * x - x) * x - x) * x -
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
     x) * x -
        x,
    double, openfhe_ckks);

TEST_EXPR_ONE_LIB(
    use_openfhe_bfv_muldep11,
    ((((((((((((x * x - x) * x - x) * x - x) * x - x) * x - x) * x - x) * x -
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
     x) * x -
        1,
    int64_t, openfhe_bfv);

TEST(TEST_DECIDION, use_concrete) {
  DagPtr dag = initDag("DECISION", 1024);
  Expr x = setInputName(dag, "x");
  Expr z = (uint8_t)2 / x;
  Valuation inputs{{"x", (uint8_t)2}};
  Valuation output = execute(inputs, dag, z);
  checkLib(dag, "concrete");
  releaseDag(dag);
}

}  // namespace iyfctest