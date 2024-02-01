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

namespace iyfctest {

std::function<double(double a, double b)> add_double = [](double a, double b) {
  return a + b;
};
std::function<int64_t(int64_t a, int64_t b)> add_int64 = [](
    int64_t a, int64_t b) { return a + b; };

std::function<double(double a, double b)> plus_double = [](double a, double b) {
  return a - b;
};
std::function<int64_t(int64_t a, int64_t b)> plus_int64 = [](
    int64_t a, int64_t b) { return a - b; };
std::function<double(double a, double b)> plus_double_inverse = [](
    double a, double b) { return b - a; };
std::function<int64_t(int64_t a, int64_t b)> plus_int64_inverse = [](
    int64_t a, int64_t b) { return b - a; };

std::function<double(double a, double b)> mul_double = [](double a, double b) {
  return a * b;
};

std::function<int64_t(int64_t a, int64_t b)> mul_int64 = [](
    int64_t a, int64_t b) { return a * b; };

template <typename T, typename... Args>
T evaluate_fun(const std::function<T(Args...)>& func, Args... args) {
  return func(args...);
}

template <typename T, typename... Args>
vector<T> plain_evaluate_two(const vector<T>& vec_input, const vector<T>& vec_b,
                             const std::function<T(Args...)>& func) {
  vector<T> vec_output;
  if (vec_input.size() != vec_b.size()) return {};
  uint32_t input_size = vec_input.size();
  for (uint32_t i = 0; i < input_size; i++) {
    vec_output.emplace_back(evaluate_fun(func, vec_input[i], vec_b[i]));
  }
  return vec_output;
}

int64_t data_bound = (1 << 8);
int vec_size = 2048;

#define INPUT_ONE_EXPR(EXPR)              \
  DagPtr dag = initDag("POLY", vec_size); \
  Expr x = setInputName(dag, "x");        \
  Expr y = EXPR;                          \
  Valuation inputs{{"x", vec_input}};

#define INPUT_TWO_EXPR(EXPR)              \
  DagPtr dag = initDag("POLY", vec_size); \
  Expr x1 = setInputName(dag, "x1");      \
  Expr x2 = setInputName(dag, "x2");      \
  Expr y = EXPR;                          \
  Valuation inputs{{"x1", vec_input}, {"x2", vec_plain}};

#define TEST_TWO_EXPR_T(NAME, PLAIN_EXPR, EXPR, TYPE)         \
  TEST(TEST_POLY, NAME) {                                     \
    vector<TYPE> vec_input;                                   \
    vector<TYPE> vec_plain;                                   \
    for (int i = 0; i < vec_size; i++) {                      \
      TYPE value1 = static_cast<TYPE>(rand() % data_bound);   \
      TYPE value2 = static_cast<TYPE>(rand() % data_bound);   \
      vec_input.emplace_back(value1);                         \
      vec_plain.emplace_back(value2);                         \
    }                                                         \
    vector<TYPE> vec_out_plain =                              \
        plain_evaluate_two(vec_input, vec_plain, PLAIN_EXPR); \
    EXPR;                                                     \
    Valuation output = execute(inputs, dag, y);               \
    check_result<TYPE>(output, vec_out_plain, 0.001);         \
    releaseDag(dag);                                          \
  }

#define TEST_TWO_EXPR_INPUT_ONE(NAME, PLAIN_EXPR, EXPR, TYPE) \
  TEST_TWO_EXPR_T(NAME, PLAIN_EXPR, INPUT_ONE_EXPR(EXPR), TYPE)
#define TEST_TWO_EXPR_INPUT_TWO(NAME, PLAIN_EXPR, EXPR, TYPE) \
  TEST_TWO_EXPR_T(NAME, PLAIN_EXPR, INPUT_TWO_EXPR(EXPR), TYPE)

//seal_ckks + - *
TEST_TWO_EXPR_INPUT_ONE(cipher_add_plain, add_double, x + vec_plain, double);
TEST_TWO_EXPR_INPUT_ONE(plain_add_cipher, add_double, vec_plain + x, double);
TEST_TWO_EXPR_INPUT_TWO(cipher_add_cipher, add_double, x1 + x2, double);
TEST_TWO_EXPR_INPUT_ONE(cipher_plus_plain, plus_double, x - vec_plain, double);
TEST_TWO_EXPR_INPUT_ONE(plain_plus_cipher, plus_double_inverse, vec_plain - x,
                        double);
TEST_TWO_EXPR_INPUT_TWO(cipher_plus_cipher, plus_double, x1 - x2, double);
TEST_TWO_EXPR_INPUT_ONE(cipher_mul_plain, mul_double, x* vec_plain, double);
TEST_TWO_EXPR_INPUT_ONE(plain_mul_cipher, mul_double, vec_plain* x, double);
TEST_TWO_EXPR_INPUT_TWO(cipher_mul_cipher, mul_double, x1* x2, double);

// seal_bfv + - *
TEST_TWO_EXPR_INPUT_ONE(cipher_add_plain_bfv, add_int64, x + vec_plain,
                        int64_t);
TEST_TWO_EXPR_INPUT_ONE(plain_add_cipher_bfv, add_int64, vec_plain + x,
                        int64_t);
TEST_TWO_EXPR_INPUT_TWO(cipher_add_cipher_bfv, add_int64, x1 + x2 + 0, int64_t);
TEST_TWO_EXPR_INPUT_ONE(cipher_plus_plain_bfv, plus_int64, x - vec_plain,
                        int64_t);
TEST_TWO_EXPR_INPUT_ONE(plain_plus_cipher_bfv, plus_int64_inverse,
                        vec_plain - x, int64_t);
TEST_TWO_EXPR_INPUT_TWO(cipher_plus_cipher_bfv, plus_int64, x1 - x2 -0,
int64_t);

TEST_TWO_EXPR_INPUT_ONE(cipher_mul_plain_bfv, mul_int64, x* vec_plain,
int64_t);
TEST_TWO_EXPR_INPUT_ONE(plain_mul_cipher_bfv, mul_int64, vec_plain* x,
int64_t);
TEST_TWO_EXPR_INPUT_TWO(cipher_mul_cipher_bfv, mul_int64, x1* x2 *1, int64_t);

TEST(TEST_POLY, cipher_poly_plain_ckks11) {
  vector<double> vec_input;
  vector<double> vec_plain;
  for (int i = 0; i < vec_size; i++) {
    vec_input.emplace_back(1);
    vec_plain.emplace_back(2);
  }
  //密文 + 明文
  DagPtr dag = initDag("add_plain", vec_size);
  Expr x = setInputName(dag, "x");
  Expr y = setInputName(dag, "y");
  Expr z =
      ((((((((((x * y - x) * x - x) * x - x) * x - x) * x - x) * x - x) * x -
          x) *
             x -
         x) *
            x -
        x) *
           x -
       x) *
          x -
      1.0;
  Valuation inputs{{"x", vec_input}, {"y", vec_plain}};
  Valuation output = execute(inputs, dag, z);
  vector<double> vec_out_plain;
  for (int i = 0; i < vec_size; i++) {
    double x = vec_input[i];
    double y = vec_plain[i];
    vec_out_plain.emplace_back(
        ((((((((((x * y - x) * x - x) * x - x) * x - x) * x - x) * x - x) * x -
            x) *
               x -
           x) *
              x -
          x) *
             x -
         x) *
            x -
        1.0);
  }
  check_result<double>(output, vec_out_plain, 1);
  releaseDag(dag);
}

TEST(TEST_POLY, cipher_poly_plain_bfv11) {
  vector<int64_t> vec_input;
  vector<int64_t> vec_plain;
  for (int i = 0; i < vec_size; i++) {
    vec_input.emplace_back(1);
    vec_plain.emplace_back(2);
  }
  // Cipher text + plain text
  DagPtr dag = initDag("add_plain", vec_size);
  Expr x = setInputName(dag, "x");
  Expr y = setInputName(dag, "y");
  Expr z =
      ((((((((((x * y - x) * x - x) * x - x) * x - x) * x - x) * x - x) * x -
          x) *
             x -
         x) *
            x -
        x) *
           x -
       x) *
          x -
      1;
  Valuation inputs{{"x", vec_input}, {"y", vec_plain}};
  Valuation output = execute(inputs, dag, z);
  vector<int64_t> vec_out_plain;
  for (int i = 0; i < vec_size; i++) {
    int64_t x = vec_input[i];
    int64_t y = vec_plain[i];
    vec_out_plain.emplace_back(
        ((((((((((x * y - x) * x - x) * x - x) * x - x) * x - x) * x - x) * x -
            x) *
               x -
           x) *
              x -
          x) *
             x -
         x) *
            x -
        1.0);
  }
  check_result<int64_t>(output, vec_out_plain, 1);
  releaseDag(dag);
}




TEST(TEST_POLY, cipher_poly_plain_bfv) {
  vector<int64_t> vec_input;
  vector<int64_t> vec_plain;
  for (int i = 0; i < vec_size; i++) {
    vec_input.emplace_back(1);
    vec_plain.emplace_back(2);
  }
  // Cipher text + plain text
  DagPtr dag = initDag("add_plain", vec_size);
  Expr x = setInputName(dag, "x");
  Expr y = setInputName(dag, "y");
  int64_t num = 1;
  Expr z = ((((((((x*y)*x)+num)*y+num)*x-num)*x-num)*x)-num*x) + num;
  Valuation inputs{{"x", vec_input}, {"y", vec_plain}};
  Valuation output = execute(inputs, dag, z);
  vector<int64_t> vec_out_plain;
  for (int i = 0; i < vec_size; i++) {
    int64_t x = vec_input[i];
    int64_t y = vec_plain[i];
    vec_out_plain.emplace_back(((((((((x*y)*x)+num)*y+num)*x-num)*x-num)*x)-num*x) + num);
  }
  check_result<int64_t>(output, vec_out_plain, 1);
  releaseDag(dag);
}
}
