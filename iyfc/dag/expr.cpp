/*
 * Implementation of Expr, overloads various C++ operators, and provides complex
 * expression implementations for basic operators such as querying and comparing.
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

#include "expr.h"

#include <algorithm>
#include <iostream>

#include "iyfc_dag.h"
#include "node.h"
#include "util/math_util.h"

namespace iyfc {
using namespace std;

Expr::~Expr() {
  // m_node will not be destructed unless the DAG is destructed
  // When temporary expression variables are destructed, nodeptr needs to be collected to ensure its lifecycle
  // m_dag->collectExprNode(m_nodeptr);

  // m_nodeptr.use_count(), m_nodeptr->m_index);
}

Expr &Expr::operator+=(const Expr &rhs) {
  this->m_nodeptr = m_dag->makeNode(OpType::Add, {m_nodeptr, rhs.m_nodeptr});
  if (!m_plaintext || !(rhs.m_plaintext)) {
    // Only when both were m_plaintext can we continue with m_plaintext
    m_plaintext = m_plaintext && rhs.m_plaintext;
  }
  return *this;
}

Expr &Expr::operator+=(const vector<double> &rhs) {
  this->m_nodeptr =
      m_dag->makeNode(OpType::Add, {m_nodeptr, m_dag->makeDenseConstant(rhs)});
  return *this;
}

Expr &Expr::operator+=(double rhs) {
  this->m_nodeptr = m_dag->makeNode(
      OpType::Add, {m_nodeptr, m_dag->makeDenseConstant(vector<double>{rhs})});
  return *this;
}

Expr &Expr::operator+=(const vector<int64_t> &rhs) {
  this->m_nodeptr = m_dag->makeNode(
      OpType::Add, {m_nodeptr, m_dag->makeInt64DenseConstant(rhs)});
  return *this;
}

Expr &Expr::operator+=(int64_t rhs) {
  this->m_nodeptr = m_dag->makeNode(
      OpType::Add,
      {m_nodeptr, m_dag->makeInt64DenseConstant(vector<int64_t>{rhs})});
  return *this;
}

Expr &Expr::operator+=(const vector<int32_t> &rhs) {
  this->m_nodeptr = m_dag->makeNode(
      OpType::Add, {m_nodeptr, m_dag->makeInt32DenseConstant(rhs)});
  return *this;
}

Expr &Expr::operator+=(int32_t rhs) {
  this->m_nodeptr = m_dag->makeNode(
      OpType::Add,
      {m_nodeptr, m_dag->makeInt32DenseConstant(vector<int32_t>{rhs})});
  return *this;
}

Expr &Expr::operator-=(const Expr &rhs) {
  this->m_nodeptr = m_dag->makeNode(OpType::Sub, {m_nodeptr, rhs.m_nodeptr});
  if (!m_plaintext || !(rhs.m_plaintext)) {
    m_plaintext = m_plaintext && rhs.m_plaintext;
  }
  return *this;
}

Expr &Expr::operator-=(const vector<double> &rhs) {
  this->m_nodeptr =
      m_dag->makeNode(OpType::Sub, {m_nodeptr, m_dag->makeDenseConstant(rhs)});
  return *this;
}
Expr &Expr::operator-=(double rhs) {
  this->m_nodeptr = m_dag->makeNode(
      OpType::Sub, {m_nodeptr, m_dag->makeDenseConstant(vector<double>{rhs})});
  return *this;
}
Expr &Expr::operator-=(const vector<int64_t> &rhs) {
  this->m_nodeptr = m_dag->makeNode(
      OpType::Sub, {m_nodeptr, m_dag->makeInt64DenseConstant(rhs)});
  return *this;
}
Expr &Expr::operator-=(int64_t rhs) {
  this->m_nodeptr = m_dag->makeNode(
      OpType::Sub,
      {m_nodeptr, m_dag->makeInt64DenseConstant(vector<int64_t>{rhs})});
  return *this;
}

Expr &Expr::operator-=(const vector<int32_t> &rhs) {
  this->m_nodeptr = m_dag->makeNode(
      OpType::Sub, {m_nodeptr, m_dag->makeInt32DenseConstant(rhs)});
  return *this;
}
Expr &Expr::operator-=(int32_t rhs) {
  this->m_nodeptr = m_dag->makeNode(
      OpType::Sub,
      {m_nodeptr, m_dag->makeInt32DenseConstant(vector<int32_t>{rhs})});
  return *this;
}

Expr &Expr::operator*=(const Expr &rhs) {
  auto new_node = m_dag->makeNode(OpType::Mul, {m_nodeptr, rhs.m_nodeptr});

  if (!m_plaintext || !(rhs.m_plaintext)) {
    this->m_plaintext = this->m_plaintext && rhs.m_plaintext;
  }
  this->m_nodeptr = new_node;
  return *this;
}

Expr &Expr::operator*=(vector<double> &rhs) {
  this->m_nodeptr =
      m_dag->makeNode(OpType::Mul, {m_nodeptr, m_dag->makeDenseConstant(rhs)});
  return *this;
}

Expr &Expr::operator*=(double rhs) {
  this->m_nodeptr = m_dag->makeNode(
      OpType::Mul, {m_nodeptr, m_dag->makeDenseConstant(vector<double>{rhs})});
  return *this;
}

Expr &Expr::operator*=(vector<int64_t> &rhs) {
  this->m_nodeptr = m_dag->makeNode(
      OpType::Mul, {m_nodeptr, m_dag->makeInt64DenseConstant(rhs)});
  return *this;
}

Expr &Expr::operator*=(int64_t rhs) {
  this->m_nodeptr = m_dag->makeNode(
      OpType::Mul,
      {m_nodeptr, m_dag->makeInt64DenseConstant(vector<int64_t>{rhs})});
  return *this;
}

Expr &Expr::operator*=(vector<int32_t> &rhs) {
  this->m_nodeptr = m_dag->makeNode(
      OpType::Mul, {m_nodeptr, m_dag->makeInt32DenseConstant(rhs)});
  return *this;
}

Expr &Expr::operator*=(int32_t rhs) {
  this->m_nodeptr = m_dag->makeNode(
      OpType::Mul,
      {m_nodeptr, m_dag->makeInt32DenseConstant(vector<int32_t>{rhs})});
  return *this;
}

Expr &Expr::operator!() {
  this->m_nodeptr = m_dag->makeNode(OpType::Negate, {m_nodeptr});
  return *this;
}

// Process comparison expressions with 7 as the base
void getCmpExprP7(const Expr &input_expr, Expr &lt_result, Expr &eq_result) {
  vector<Expr> lt_list;
  vector<Expr> eq_list;
  /*
  # p is the base of decomposition. The larger p is, the shorter the vector after integer decomposition, and the more numbers can be sorted or compared.
    # ==================================================================================
    # p=3 || Max number for 32-bit sorting: 16 || Max number for 32-bit comparison: 256
    # p=5 || Max number for 32-bit sorting: 20 || Max number for 32-bit comparison: 405
    # p=7 || Max number for 32-bit sorting: 22 || Max number for 32-bit comparison: 512
    # ==================================================================================
      p==3:
        LT_decompose_list[0] = (2*z**2 + z)
        Eq_decompose_list[0] = (1-z) * (1+z)
     p==5:
        LT_decompose_list[0] = (3*z**4 + 3*z**3 + 4*z)
        Eq_decompose_list[0] = (1-z) * (1+z) * (1-z) * (1+z)
     p==7:
        LT_decompose_list[0] = (4*z**6 + 6*z**5 + z**3 + 3*z)
        Eq_decompose_list[0] = (1-z) * (1+z) * (1-z) * (1+z) * (1-z) * (1+z)

  Here, p is initially set to 3 by default.
  */
  // lt_list.emplace_back(input_expr_z * input_expr_z * 2.0 + input_expr_z);
  // eq_list.emplace_back((1.0 - input_expr_z) * (1.0 + input_expr_z));

  // p = 7
  lt_list.emplace_back((1.0 / 72.0 * input_expr * input_expr +
                        3.0 / 40.0 * input_expr + 37.0 / 360.0) *
                       input_expr * (input_expr - 1.0) * (input_expr - 2.0) *
                       (input_expr - 3.0));
  eq_list.emplace_back((-1.0) / 36.0 * (input_expr + 3.0) * (input_expr + 2.0) *
                       (input_expr + 1.0) * (input_expr - 1.0) *
                       (input_expr - 2.0) * (input_expr - 3.0));

  Expr lt_r = lt_list[0] << 1;
  Expr eq_r = eq_list[0] << 1;
  lt_list.emplace_back((lt_list[0] + lt_r * eq_list[0]));
  eq_list.emplace_back((eq_list[0] * eq_r));

  lt_r = lt_list[1] << 2;
  eq_r = eq_list[1] << 2;
  lt_list.emplace_back((lt_list[1] + lt_r * eq_list[1]));
  eq_list.emplace_back((eq_list[1] * eq_r));

  lt_r = lt_list[2] << 4;
  eq_r = eq_list[2] << 4;
  lt_list.emplace_back((lt_list[2] + lt_r * eq_list[2]));
  eq_list.emplace_back((eq_list[2] * eq_r));

  lt_r = lt_list[3] << 8;
  eq_r = eq_list[3] << 8;
  lt_list.emplace_back((lt_list[3] + lt_r * eq_list[3]));
  eq_list.emplace_back((eq_list[3] * eq_r));
  eq_result = eq_list[4];
  lt_result = lt_list[4];
}

void getCmpExprP3(const Expr &input_expr, Expr &lt_result, Expr &eq_result) {
  uint32_t d = 2, l_first = 2, l_second = 2, l_third = 2, l_fourth = 2;
  vector<Expr> lt_list;
  vector<Expr> eq_list;
  lt_list.emplace_back(0.5 * input_expr * input_expr - 0.5 * input_expr);

  eq_list.emplace_back((1.0 - input_expr) * (1.0 + input_expr));

  // Calculate the final comparison result using the idea of 5 layers to reduce multiplication depth
  // Combine to obtain the first-layer comparison result based on LT_decompose_list[0] and Eq_decompose_list[0]
  for (uint32_t i = 0; i < d - 1; i++) {
    Expr lt_r = lt_list[i] << 1;
    Expr eq_r = eq_list[i] << 1;
    lt_list.emplace_back((lt_list[i] + lt_r * eq_list[i]));
    eq_list.emplace_back((eq_list[i] * eq_r));
  }
  // LT_first_list[0] is used to store the less-than result of the first level
  // Eq_first_list[0] is used to store the equal results of the first level
  vector<Expr> lt_list_first;
  vector<Expr> eq_list_first;
  lt_list_first.emplace_back(lt_list[d - 1]);
  eq_list_first.emplace_back(eq_list[d - 1]);

  // Second layer
  for (uint32_t i = 0; i < l_first - 1; i++) {
    Expr lt_r = lt_list_first[i] << d;
    Expr eq_r = eq_list_first[i] << d;
    lt_list_first.emplace_back((lt_list_first[i] + lt_r * eq_list_first[i]));
    eq_list_first.emplace_back((eq_list_first[i] * eq_r));
  }
  // Same as above
  vector<Expr> lt_list_second;
  vector<Expr> eq_list_second;
  lt_list_second.emplace_back(lt_list_first[l_first - 1]);
  eq_list_second.emplace_back(eq_list_first[l_first - 1]);
  for (uint32_t i = 0; i < l_second - 1; i++) {
    Expr lt_r = lt_list_second[i] << d * l_first;
    Expr eq_r = eq_list_second[i] << d * l_first;
    lt_list_second.emplace_back((lt_list_second[i] + lt_r * eq_list_second[i]));
    eq_list_second.emplace_back((eq_list_second[i] * eq_r));
  }
  // Same as above
  vector<Expr> lt_list_third;
  vector<Expr> eq_list_third;
  lt_list_third.emplace_back(lt_list_second[l_second - 1]);
  eq_list_third.emplace_back(eq_list_second[l_second - 1]);
  for (uint32_t i = 0; i < l_third - 1; i++) {
    Expr lt_r = lt_list_third[i] << d * l_first * l_second;
    Expr eq_r = eq_list_third[i] << d * l_first * l_second;
    lt_list_third.emplace_back((lt_list_third[i] + lt_r * eq_list_third[i]));
    eq_list_third.emplace_back((eq_list_third[i] * eq_r));
  }
  // Same as above
  vector<Expr> lt_list_fourth;
  vector<Expr> eq_list_fourth;
  lt_list_fourth.emplace_back(lt_list_third[l_third - 1]);
  eq_list_fourth.emplace_back(eq_list_third[l_third - 1]);
  for (uint32_t i = 0; i < l_fourth - 1; i++) {
    Expr lt_r = lt_list_fourth[i] << d * l_first * l_second * l_third;
    Expr eq_r = eq_list_fourth[i] << d * l_first * l_second * l_third;
    lt_list_fourth.emplace_back((lt_list_fourth[i] + lt_r * eq_list_fourth[i]));
    eq_list_fourth.emplace_back((eq_list_fourth[i] * eq_r));
  }

  eq_result = eq_list_fourth[l_fourth - 1];
  lt_result = lt_list_fourth[l_fourth - 1];
}

// Construct a comparison expression
Expr CmpOpHelper(const Expr &lhs, const Expr &rhs, CMP_TYPE type) {
  // The slot size is currently fixed at 16384
  lhs.m_dag->setVecSize(CMP_DAG_SIZE);

  Expr input_expr_z = lhs - rhs;
  Expr lt_result;
  Expr eq_result;

  if (CMP_P == 3) {
    getCmpExprP3(input_expr_z, lt_result, eq_result);
  } else {
    getCmpExprP7(input_expr_z, lt_result, eq_result);
  }

  std::vector<double> vec_mask;
  getMaskVec(CMP_BIT_LEN, CMP_DAG_SIZE, vec_mask);
  // Fill the 32 bits of the result
  std::vector<Expr> expr_list_mask;

  if (type == EQ) {
    Expr out_after_mask = vec_mask * eq_result;
    expr_list_mask.emplace_back(out_after_mask);
  } else if (type == LESS) {
    Expr out_after_mask = vec_mask * lt_result;
    expr_list_mask.emplace_back(out_after_mask);
  }

  for (int i = 0; i < CMP_BIT_LEN - 1; i++) {
    expr_list_mask.emplace_back(expr_list_mask[i] >> 1);
  }
  Expr out_result = expr_list_mask[0];

  for (int i = 0; i < CMP_BIT_LEN - 1; i++) {
    out_result = out_result + expr_list_mask[i + 1];
  }

  return out_result;
}

// Handles the expression for comparing ciphertext with plaintext
Expr plaintToCmpExpr(DagPtr dag, uint32_t ul_num) {
  dag->setVecSize(CMP_DAG_SIZE);

  uint32_t p = CMP_P;
  uint32_t composemod = (p - 1) / 2 + 1;
  // Set the polynomial dimension (currently fixed at 16384)
  vector<double> compose_vec_x;
  uint32_t bits = CMP_BIT_LEN;

  vector<uint32_t> tmp_vec = decimalConvert(ul_num, composemod, bits);
  for (int i = 0; i < MAX_CMP_NUM; i++)
    compose_vec_x.insert(compose_vec_x.end(), tmp_vec.begin(), tmp_vec.end());

  compose_vec_x.resize(CMP_DAG_SIZE);

  return Expr(dag, compose_vec_x);
}

Expr IYFC_SO_EXPORT operator==(const Expr &lhs, const Expr &rhs) {
  return CmpOpHelper(lhs, rhs, EQ);
}

Expr IYFC_SO_EXPORT operator==(const Expr &lhs, uint32_t plain_num) {
  return CmpOpHelper(lhs, plaintToCmpExpr(lhs.m_dag, plain_num), EQ);
}

// 1- eq
Expr IYFC_SO_EXPORT operator!=(const Expr &lhs, const Expr &rhs) {
  return double(1.0) - (lhs == rhs);
}
Expr IYFC_SO_EXPORT operator!=(const Expr &lhs, uint32_t plain_num) {
  return double(1.0) - (lhs == plain_num);
}

Expr IYFC_SO_EXPORT operator<(const Expr &lhs, const Expr &rhs) {
  return CmpOpHelper(lhs, rhs, LESS);
}

Expr IYFC_SO_EXPORT operator<(const Expr &lhs, uint32_t plain_num) {
  return CmpOpHelper(lhs, plaintToCmpExpr(lhs.m_dag, plain_num), LESS);
}

// eq + lt
Expr IYFC_SO_EXPORT operator<=(const Expr &lhs, const Expr &rhs) {
  return CmpOpHelper(lhs, rhs, EQ) + CmpOpHelper(lhs, rhs, LESS);
}
Expr IYFC_SO_EXPORT operator<=(const Expr &lhs, uint32_t plain_num) {
  Expr tmp = plaintToCmpExpr(lhs.m_dag, plain_num);
  return CmpOpHelper(lhs, tmp, EQ) + CmpOpHelper(lhs, tmp, LESS);
}

// 1- <=
Expr IYFC_SO_EXPORT operator>(const Expr &lhs, const Expr &rhs) {
  return 1.0 - (lhs <= rhs);
}

Expr IYFC_SO_EXPORT operator>(const Expr &lhs, uint32_t plain_num) {
  return 1.0 - (lhs <= plain_num);
}

// 1- lt
Expr IYFC_SO_EXPORT operator>=(const Expr &lhs, const Expr &rhs) {
  return 1.0 - (lhs < rhs);
}
Expr IYFC_SO_EXPORT operator>=(const Expr &lhs, uint32_t plain_num) {
  return 1.0 - (lhs < plain_num);
}

Expr IYFC_SO_EXPORT operator&&(const Expr &lhs, const Expr &rhs) {
  // Multiple condition queries  Reduce scale  Try to use the SEAL library
  if (lhs.m_dag->m_try_reduce_scale_cnt--) {
    lhs.m_dag->m_scale = lhs.m_dag->m_scale - REDUCE_SCALE;
  }

  return lhs * rhs;
}
Expr IYFC_SO_EXPORT operator||(const Expr &lhs, const Expr &rhs) {
  // Multiple condition queries  Reduce scale  Try to use the SEAL library
  if (lhs.m_dag->m_try_reduce_scale_cnt--) {
    lhs.m_dag->m_scale = lhs.m_dag->m_scale - REDUCE_SCALE;
  }

  return 1.0 - (1.0 - lhs) * (1.0 - rhs);
}

//+
Expr IYFC_SO_EXPORT operator+(const Expr &lhs, const Expr &rhs) {
  auto new_node =
      lhs.m_dag->makeNode(OpType::Add, {lhs.m_nodeptr, rhs.m_nodeptr});
  return Expr(lhs.m_dag, new_node);
}

Expr IYFC_SO_EXPORT operator+(const Expr &lhs, double lf) {
  return lhs + Expr(lhs.m_dag, lf);
}

Expr IYFC_SO_EXPORT operator+(double lf, const Expr &rhs) {
  return Expr(rhs.m_dag, lf) + rhs;
}

Expr IYFC_SO_EXPORT operator+(const Expr &lhs, const vector<double> &vec_rhs) {
  return lhs + Expr(lhs.m_dag, vec_rhs);
}

Expr IYFC_SO_EXPORT operator+(const vector<double> &lhs, const Expr &rhs) {
  return Expr(rhs.m_dag, lhs) + rhs;
}

Expr IYFC_SO_EXPORT operator+(const Expr &lhs, int32_t lf) {
  return lhs + Expr(lhs.m_dag, lf);
}

Expr IYFC_SO_EXPORT operator+(int32_t lf, const Expr &rhs) {
  return Expr(rhs.m_dag, lf) + rhs;
}

Expr IYFC_SO_EXPORT operator+(const Expr &lhs, const vector<int64_t> &vec_rhs) {
  return lhs + Expr(lhs.m_dag, vec_rhs);
}

Expr IYFC_SO_EXPORT operator+(const vector<int64_t> &lhs, const Expr &rhs) {
  return Expr(rhs.m_dag, lhs) + rhs;
}

Expr IYFC_SO_EXPORT operator+(const Expr &lhs, int64_t lf) {
  return lhs + Expr(lhs.m_dag, lf);
}

Expr IYFC_SO_EXPORT operator+(int64_t lf, const Expr &rhs) {
  return Expr(rhs.m_dag, lf) + rhs;
}

Expr IYFC_SO_EXPORT operator+(const Expr &lhs, const vector<int32_t> &vec_rhs) {
  return lhs + Expr(lhs.m_dag, vec_rhs);
}

Expr IYFC_SO_EXPORT operator+(const vector<int32_t> &lhs, const Expr &rhs) {
  return Expr(rhs.m_dag, lhs) + rhs;
}

//-
Expr IYFC_SO_EXPORT operator-(const Expr &lhs, const Expr &rhs) {
  auto new_node =
      lhs.m_dag->makeNode(OpType::Sub, {lhs.m_nodeptr, rhs.m_nodeptr});
  return Expr(lhs.m_dag, new_node);
}

Expr IYFC_SO_EXPORT operator-(const Expr &lhs, double lf) {
  return lhs - Expr(lhs.m_dag, lf);
}

Expr IYFC_SO_EXPORT operator-(double lf, const Expr &rhs) {
  return Expr(rhs.m_dag, lf) - rhs;
}

Expr IYFC_SO_EXPORT operator-(const Expr &lhs, const vector<double> &vec_rhs) {
  return lhs - Expr(lhs.m_dag, vec_rhs);
}

Expr IYFC_SO_EXPORT operator-(const vector<double> &lhs, const Expr &rhs) {
  return Expr(rhs.m_dag, lhs) - rhs;
}

Expr IYFC_SO_EXPORT operator-(const Expr &lhs, int32_t lf) {
  return lhs - Expr(lhs.m_dag, lf);
}

Expr IYFC_SO_EXPORT operator-(int32_t lf, const Expr &rhs) {
  return Expr(rhs.m_dag, lf) - rhs;
}

Expr IYFC_SO_EXPORT operator-(const Expr &lhs, int64_t lf) {
  return lhs - Expr(lhs.m_dag, lf);
}

Expr IYFC_SO_EXPORT operator-(int64_t lf, const Expr &rhs) {
  return Expr(rhs.m_dag, lf) - rhs;
}

Expr IYFC_SO_EXPORT operator-(const Expr &lhs, const vector<int64_t> &vec_rhs) {
  return lhs - Expr(lhs.m_dag, vec_rhs);
}

Expr IYFC_SO_EXPORT operator-(const Expr &lhs, const vector<int32_t> &rhs) {
  return lhs - Expr(lhs.m_dag, rhs);
}

Expr IYFC_SO_EXPORT operator-(const vector<int64_t> &lhs, const Expr &rhs) {
  return Expr(rhs.m_dag, lhs) - rhs;
}

Expr IYFC_SO_EXPORT operator-(const vector<int32_t> &lhs, const Expr &rhs) {
  return Expr(rhs.m_dag, lhs) - rhs;
}

//*
Expr IYFC_SO_EXPORT operator*(const Expr &lhs, const Expr &rhs) {
  auto new_node =
      lhs.m_dag->makeNode(OpType::Mul, {lhs.m_nodeptr, rhs.m_nodeptr});
  return Expr(lhs.m_dag, new_node);
}
Expr IYFC_SO_EXPORT operator*(const Expr &lhs, double lf) {
  return lhs * Expr(lhs.m_dag, lf);
}

Expr IYFC_SO_EXPORT operator*(double lf, const Expr &rhs) {
  return Expr(rhs.m_dag, lf) * rhs;
}

Expr IYFC_SO_EXPORT operator*(const Expr &lhs, const vector<double> &vec_rhs) {
  return lhs * Expr(lhs.m_dag, vec_rhs);
}

Expr IYFC_SO_EXPORT operator*(const vector<double> &lhs, const Expr &rhs) {
  return Expr(rhs.m_dag, lhs) * rhs;
}

Expr IYFC_SO_EXPORT operator*(const Expr &lhs, int32_t lf) {
  return lhs * Expr(lhs.m_dag, lf);
}

Expr IYFC_SO_EXPORT operator*(int32_t lf, const Expr &rhs) {
  return Expr(rhs.m_dag, lf) * rhs;
}

Expr IYFC_SO_EXPORT operator*(const Expr &lhs, const vector<int64_t> &vec_rhs) {
  return lhs * Expr(lhs.m_dag, vec_rhs);
}

Expr IYFC_SO_EXPORT operator*(const vector<int64_t> &lhs, const Expr &rhs) {
  return Expr(rhs.m_dag, lhs) * rhs;
}

Expr IYFC_SO_EXPORT operator*(const Expr &lhs, int64_t lf) {
  return lhs * Expr(lhs.m_dag, lf);
}

Expr IYFC_SO_EXPORT operator*(int64_t lf, const Expr &rhs) {
  return Expr(rhs.m_dag, lf) * rhs;
}

Expr IYFC_SO_EXPORT operator*(const Expr &lhs, const vector<int32_t> &vec_rhs) {
  return lhs * Expr(lhs.m_dag, vec_rhs);
}

Expr IYFC_SO_EXPORT operator*(const vector<int32_t> &lhs, const Expr &rhs) {
  return Expr(rhs.m_dag, lhs) * rhs;
}

/**
 * @brief Division Helper Function - Version 1
 * @details The divisor must be in the range (P_M/2, P_M).
 */
Expr div_hepler_p(const Expr &rhs) {
  const int P_DIV_LEN = 32;
  const uint64_t P_M = pow(2, P_DIV_LEN);

  Expr expr_y = 1.0 - pow(2.0, -P_DIV_LEN) * rhs;
  Expr expr_y_2 = expr_y * expr_y;
  Expr expr_y_4 = expr_y_2 * expr_y_2;
  Expr expr_y_8 = expr_y_4 * expr_y_4;
  Expr expr_y_16 = expr_y_8 * expr_y_8;
  Expr expr_y_32 = expr_y_16 * expr_y_16;

  Expr div_out = pow(2.0, -P_DIV_LEN) * (1.0 + expr_y) * (1.0 + expr_y_2) *
                 (1.0 + expr_y_4) * (1.0 + expr_y_8) * (1.0 + expr_y_16) *
                 (1.0 + expr_y_32);
}

/**
 * @brief Division Helper Function - Version 2
 * @details The divisor must be in the range (0, 2).
 */
Expr div_hepler_2(const Expr &rhs) {
  const int D_RANGE = 5;
  const double DIV_V2_MIN = 0.0;
  const double DIV_V2_MAX = 1024;
  const double DIV_V2_A = 2.0 / (DIV_V2_MAX - DIV_V2_MIN);
  // Normalize the divisor to the range (0, 2)
  Expr tmp_rhs = DIV_V2_A * rhs;

  Expr expr_a = 2.0 - tmp_rhs;
  Expr expr_b = 1.0 - tmp_rhs;
  for (int i = 0; i < D_RANGE; i++) {
    expr_b = expr_b * expr_b;
    expr_a = expr_a * (1.0 + expr_b);
  }
  // Restore the result to the original scale
  // When min is 0, expr_a * DIV_V2_A is the restored value
  Expr expr_inv = expr_a * DIV_V2_A;
  return expr_inv;
}

// Experience concrete division
Expr IYFC_SO_EXPORT operator/(uint8_t lhs, const Expr &rhs) {
  rhs.m_dag->setSupportShortInt(true);
  return Expr(rhs.m_dag, lhs) / rhs;
}

Expr IYFC_SO_EXPORT operator/(const Expr &lhs, uint8_t rhs) {
  lhs.m_dag->setSupportShortInt(true);
  return lhs / Expr(lhs.m_dag, rhs);
}

Expr IYFC_SO_EXPORT operator/(const Expr &lhs, const Expr &rhs) {
  // Concret TFHE algorithm directly invokes division capability, supporting up to shortint4.
  if (lhs.m_dag->supportShortInt()) {
    auto new_node =
        lhs.m_dag->makeNode(OpType::Div, {lhs.m_nodeptr, rhs.m_nodeptr});
    return Expr(lhs.m_dag, new_node);
  } else {
    return lhs * div_hepler_2(rhs);
  }
}
Expr IYFC_SO_EXPORT operator/(const Expr &lhs, double rhs) {
  return lhs * (1 / double(rhs));
}

Expr IYFC_SO_EXPORT operator/(double lhs, const Expr &rhs) {
  return Expr(rhs.m_dag, lhs) / rhs;
}
Expr IYFC_SO_EXPORT operator/(const vector<double> &lhs, const Expr &rhs) {
  return Expr(rhs.m_dag, lhs) / rhs;
}

// Query statement using rotations to compute sum and count
Expr SumCntHelper(const Expr &lhs) {
  lhs.m_dag->setVecSize(CMP_DAG_SIZE);
  Expr sum_expr = lhs;
  uint32_t total_num = lhs.m_dag->getNumSize();

  // If the total number is not MAX_CMP_NUM, perform rotations directly
  if (total_num != MAX_CMP_NUM) {
    for (int i = 1; i <= total_num - 1; i++) {
      sum_expr += lhs << (i * FFT_N);
    }
  } else {
    // Reduce rotation keys
    for (int i = MAX_CMP_NUM / 2; i >= 1;) {
      sum_expr += sum_expr << (i * FFT_N);
      i /= 2;
    }
  }

  std::vector<double> vec_mask;
  getSumMaskVec(FFT_N, CMP_DAG_SIZE, vec_mask);
  return (sum_expr * vec_mask);
}

Expr IYFC_SO_EXPORT QueryRow(const Expr &lhs, const Expr &rhs) {
  return lhs * (rhs);
}

Expr IYFC_SO_EXPORT QuerySum(const Expr &lhs, const Expr &rhs) {
  return SumCntHelper(lhs * rhs);
}

Expr IYFC_SO_EXPORT QueryCnt(const Expr &lhs) { return SumCntHelper(lhs); }

/*Multiplication depth is not enough*/
// Expr  QueryAvg(const Expr &lhs, const Expr &rhs) {
//   return QuerySum / SumCntHelper(rhs);
// }

Expr IYFC_SO_EXPORT operator<<(const Expr &lhs, uint32_t k) {
  if (true || k == 1 || k == 2) {
    auto new_node = lhs.m_dag->makeLeftRotation(lhs.m_nodeptr, k);
    return Expr(lhs.m_dag, new_node);

  } else {
    // Theoretically, for all K, it can be reduced to 2 or 1 rotations.
    int loop = k / 2;
    Expr ro_expr = lhs;
    for (int i = 0; i < loop; i++) {
      ro_expr = ro_expr << 2;
    }
    if (k % 2 != 0) ro_expr = ro_expr << 1;
    return ro_expr;
  }
}

Expr IYFC_SO_EXPORT operator>>(const Expr &lhs, uint32_t k) {
  if (true || k == 1 || k == 2) {
    auto new_node = lhs.m_dag->makeRightRotation(lhs.m_nodeptr, k);
    return Expr(lhs.m_dag, new_node);
  } else {
    int loop = k / 2;
    Expr ro_expr = lhs;
    for (int i = 0; i < loop; i++) {
      ro_expr = ro_expr >> 2;
    }
    if (k % 2 != 0) ro_expr = ro_expr >> 1;
    return ro_expr;
  }
}

Expr &Expr::operator++() {
  Expr one = Expr(m_dag, 1.0);
  return (*this) += one;
}

Expr &Expr::operator--() {
  Expr one = Expr(m_dag, 1.0);
  return (*this) -= one;
}
/*
Expr::Expr(std::shared_ptr<Dag> dag, double lf)
    : m_dag(dag), m_nodeptr(dag->makeDenseConstant(vector<double>{lf})) {
  this->m_plaintext = true;
  this->m_multdepth = 0;
}

Expr::Expr(std::shared_ptr<Dag> dag, uint32_t u_value)
    : m_dag(dag), m_nodeptr(dag->makeUint32Const(u_value)) {
  this->m_plaintext = true;
  this->m_multdepth = 0;
}

Expr::Expr(std::shared_ptr<Dag> dag, NodePtr nodeptr)
    : m_dag(dag), m_nodeptr(nodeptr) {
  this->m_plaintext = true;
  this->m_multdepth = 0;
}

Expr::Expr(std::shared_ptr<Dag> dag, const vector<double> &values)
    : m_dag(dag), m_nodeptr(dag->makeDenseConstant(values)) {
  this->m_multdepth = 0;
}
*/

IYFC_SO_EXPORT Expr::Expr(const Expr &other) : m_dag(other.m_dag) {
  m_multdepth = other.m_multdepth;
  m_plaintext = other.m_plaintext;
  m_nodeptr = other.m_nodeptr;
}

IYFC_SO_EXPORT Expr::Expr(Expr &&other) : m_dag(other.m_dag) {
  m_multdepth = other.m_multdepth;
  m_plaintext = other.m_plaintext;
  m_nodeptr = other.m_nodeptr;
}

IYFC_SO_EXPORT Expr &Expr::operator=(const Expr &other) {
  m_dag = other.m_dag;
  m_multdepth = other.m_multdepth;
  m_plaintext = other.m_plaintext;
  m_nodeptr = other.m_nodeptr;
  return *this;
}

// IYFC_SO_EXPORT Expr &Expr::operator=(Expr &&other) {
//   m_dag = other.m_dag;

//   m_multdepth = other.m_multdepth;
//   m_nodeptr = other.m_nodeptr;

//   return *this;
// }

IYFC_SO_EXPORT Expr::Expr(Dag *dag, double lf)
    : m_dag(dag), m_nodeptr(dag->makeDenseConstant(vector<double>{lf})) {
  this->m_plaintext = true;
  this->m_multdepth = 0;
}

IYFC_SO_EXPORT Expr::Expr(Dag *dag, int32_t lf)
    : m_dag(dag), m_nodeptr(dag->makeInt64DenseConstant(vector<int64_t>{lf})) {
  this->m_plaintext = true;
  this->m_multdepth = 0;
}

IYFC_SO_EXPORT Expr::Expr(Dag *dag, int64_t lf)
    : m_dag(dag), m_nodeptr(dag->makeInt64DenseConstant(vector<int64_t>{lf})) {
  this->m_plaintext = true;
  this->m_multdepth = 0;
}

IYFC_SO_EXPORT Expr::Expr(Dag *dag, uint8_t u_value) : m_dag(dag) {
  m_nodeptr = dag->makeUint32Const(u_value);
  this->m_plaintext = true;
  this->m_multdepth = 0;
}

IYFC_SO_EXPORT Expr::Expr(Dag *dag, NodePtr nodeptr)
    : m_dag(dag), m_nodeptr(nodeptr) {
  this->m_plaintext = true;
  this->m_multdepth = 0;
}

IYFC_SO_EXPORT Expr::Expr(Dag *dag, const vector<double> &values)
    : m_dag(dag), m_nodeptr(dag->makeDenseConstant(values)) {
  this->m_multdepth = 0;
}

IYFC_SO_EXPORT Expr::Expr(Dag *dag, const vector<int64_t> &values)
    : m_dag(dag), m_nodeptr(dag->makeInt64DenseConstant(values)) {
  this->m_multdepth = 0;
}

IYFC_SO_EXPORT Expr::Expr(Dag *dag, const vector<int32_t> &values)
    : m_dag(dag), m_nodeptr(dag->makeInt32DenseConstant(values)) {
  this->m_multdepth = 0;
}

IYFC_SO_EXPORT Expr::Expr() {}

}  // namespace iyfc
