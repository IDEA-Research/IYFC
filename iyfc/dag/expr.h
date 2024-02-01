/*
 * Expression Definition
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
#pragma once
#include <functional>
#include <string>
#include <vector>

#include "comm_include.h"

using namespace std;

namespace iyfc {
class Dag;

/**
 * @class Expr
 * @brief Core class for expressions, overloads + - * / ++ <
 * Constructs corresponding nodes through expressions and eventually combines them into a DAG.
 */
class IYFC_SO_EXPORT Expr {
 public:
  ~Expr();

  /**
   * @brief Default constructor for Expr
   */
  Expr();

  /**
   * @brief Copy constructor for Expr
   */
  Expr(const Expr &other);

  /**
   * @brief Move constructor for Expr
   */
  Expr(Expr &&other);

  /**
   * @brief Copy assignment for Expr
   */
  Expr &operator=(const Expr &other);

  /**
   * @brief Move assignment for Expr (default)
   */
  Expr &operator=(Expr &&other) = default;

  /* Plaintext constructor
  Expr(std::shared_ptr<Dag> dag, NodePtr nodeptr);

  Expr(std::shared_ptr<Dag> dag, double lf);

  Expr(std::shared_ptr<Dag> dag, uint8_t u8_const);

  Expr(std::shared_ptr<Dag> dag, const vector<double> &values);
  */

  /**
   * @brief Constructor for Expr using a DAG and NodePtr
   * @param[in] dag The DAG to which the node belongs
   * @param[in] nodeptr The node pointer
   */
  Expr(Dag *dag, NodePtr nodeptr);

  /**
   * @brief Constructor for Expr using a DAG and a double value
   * @param[in] dag The DAG to which the expression belongs
   * @param[in] lf The double-precision plaintext value
   */
  Expr(Dag *dag, double lf);

  /**
   * @brief Constructor for Expr using a DAG and an int32_t value
   * @details To avoid ambiguity in plaintext expressions, where int64_t and double types have the same priority,
   *          and to prevent ambiguity with Expr(p_dag, 1), which can be interpreted as either int32_t or int64_t,
   *          we use int32_t overloads here to avoid ambiguity in plaintext expressions, with int32_t taking priority.
   * @param[in] dag The DAG to which the expression belongs
   * @param[in] i32 The int32_t plaintext value
   */
  Expr(Dag *dag, int32_t i32);

  /**
   * @brief Constructor for Expr using a DAG and an int64_t value
   * @param[in] dag The DAG to which the expression belongs
   * @param[in] i64 The int64_t plaintext value
   */
  Expr(Dag *dag, int64_t i64);

  /**
   * @brief Constructor for Expr using a DAG and a uint8_t value
   * @details Concrete supports uint8 type
   * @param[in] dag The DAG to which the expression belongs
   * @param[in] u8_const The uint8_t value
   */
  Expr(Dag *dag, uint8_t u8_const);

  /**
   * @brief Constructor for Expr using a DAG and a vector<double> value
   * @param[in]  dag The DAG to which the expression belongs
   * @param[in]  values The vector<double> value
   */
  Expr(Dag *dag, const vector<double> &values);

  /**
   * @brief Constructor for Expr using a DAG and a vector<int64_t> values
   * @param[in] dag The DAG to which the expression belongs
   * @param[in] values The vector<int64_t> value
   */
  Expr(Dag *dag, const vector<int64_t> &values);

  /**
   * @brief Constructor for Expr using a DAG and a vector<int32_t> value
   * @param[in] dag The DAG to which the expression belongs
   * @param[in] values The vector<int32_t> value
   */
  Expr(Dag *dag, const vector<int32_t> &values);

  /**
   * @brief Overloaded operator += for addition-and-assign
   * @param[in]  rhs Operand of type Expr&, vector<double>, vector<int64_t>, vector<int32_t>, double, int64_t, int32_t, and other data types
   */
  Expr &operator+=(const Expr &rhs);
  Expr &operator+=(const vector<double> &rhs);
  Expr &operator+=(const vector<int64_t> &rhs);
  Expr &operator+=(const vector<int32_t> &rhs);
  Expr &operator+=(double rhs);
  Expr &operator+=(int64_t rhs);
  Expr &operator+=(int32_t rhs);

  /**
   * @brief Overloaded operator -= for subtraction-and-assign
   * @param[in] rhs Operand of type Expr&, vector<double>, vector<int64_t>, vector<int32_t>, double, int64_t, int32_t, and other data types
   */
  Expr &operator-=(const Expr &rhs);
  Expr &operator-=(const vector<double> &rhs);
  Expr &operator-=(const vector<int64_t> &rhs);
  Expr &operator-=(const vector<int32_t> &rhs);
  Expr &operator-=(double rhs);
  Expr &operator-=(int64_t rhs);
  Expr &operator-=(int32_t rhs);

  /**
   * @brief Overloaded operator *= for multiplication-and-assign
   * @param[in] rhs Operand of type Expr&, vector<double>, vector<int64_t>, vector<int32_t>, double, int64_t, int32_t, and other data types
   */
  Expr &operator*=(const Expr &rhs);
  Expr &operator*=(vector<double> &rhs);
  Expr &operator*=(vector<int64_t> &rhs);
  Expr &operator*=(vector<int32_t> &rhs);
  Expr &operator*=(double rhs);
  Expr &operator*=(int64_t rhs);
  Expr &operator*=(int32_t rhs);

  /// Increment operator
  Expr &operator++();

  /// Decrement operator
  Expr &operator--();

  /// Increment operator
  Expr operator++(int) {
    auto tmp = *this;
    operator++();
    return tmp;
  }

  /// Decrement operator
  Expr operator--(int) {
    auto tmp = *this;
    operator--();
    return tmp;
  }

  /// Logical negation operator
  Expr &operator!();

  /**
   * @brief Overloaded operator == for ciphertext-ciphertext comparison
   * @details The operands, lhs and rhs, need to be expressions that have already undergone base decomposition.
   * @param[in] lhs Expr
   * @param[in] rhs Expr
   * @return Expr The result of the equality operation. The slot is stored as a ciphertext state of 0/1, representing equal/not equal.
   */
  friend Expr operator==(const Expr &lhs, const Expr &rhs);
  /**
   * @brief Overloaded operator == for ciphertext-plaintext comparison
   * @details The operands, lhs and rhs, need to be expressions that have already undergone base decomposition.
   * @param[in] lhs Expr
   * @param[in] plain_num uint32_t The plaintext number
   * @return Expr The result of the equality operation. The slot is stored as a ciphertext state of 0/1, representing equal/not equal.
   */
  friend Expr operator==(const Expr &lhs, uint32_t plain_num);

  /**
   * @brief Overloaded operator != for ciphertext-ciphertext comparison
   * @details The operands, lhs and rhs, need to be expressions that have already undergone base decomposition.
   * @param[in] lhs Expr
   * @param[in] rhs Expr
   * @return Expr The result of the inequality operation. The slot is stored as a ciphertext state of 0/1.
   */
  friend Expr operator!=(const Expr &lhs, const Expr &rhs);
  /**
   * @brief Overloaded operator != for ciphertext-plaintext comparison
   * @details The operands, lhs and rhs, need to be expressions that have already undergone base decomposition.
   * @param[in] lhs Expr
   * @param[in] plain_num uint32_t The plaintext number
   * @return Expr The result of the inequality operation. The slot is stored as a ciphertext state of 0/1.
   */
  friend Expr operator!=(const Expr &lhs, uint32_t plain_num);

  /**
   * @brief Overloaded operator >= for ciphertext-ciphertext comparison
   * @details The operands, lhs and rhs, need to be expressions that have already undergone base decomposition.
   * @param[in] lhs Expr
   * @param[in] rhs Expr
   * @return Expr The result of the greater-than-or-equal operation. The slot is stored as a ciphertext state of 0/1.
   */
  friend Expr operator>=(const Expr &lhs, const Expr &rhs);
  /**
   * @brief Overloaded operator >= for ciphertext-plaintext comparison
   * @details The operands, lhs and rhs, need to be expressions that have already undergone base decomposition.
   * @param[in] lhs Expr
   * @param[in] plain_num uint32_t The plaintext number
   * @return Expr The result of the greater-than-or-equal operation. The slot is stored as a ciphertext state of 0/1.
   */
  friend Expr operator>=(const Expr &lhs, uint32_t plain_num);

  /**
   * @brief Overloaded operator > for ciphertext-ciphertext comparison
   * @details The operands, lhs and rhs, need to be expressions that have already undergone base decomposition.
   * @param[in] lhs Expr
   * @param[in] rhs Expr
   * @return Expr The result of the greater-than operation. The slot is stored as a ciphertext state of 0/1.
   */
  friend Expr operator>(const Expr &lhs, const Expr &rhs);
  /**
   * @brief Overloaded operator > for ciphertext-plaintext comparison
   * @details The operands, lhs and rhs, need to be expressions that have already undergone base decomposition.
   * @param[in] lhs Expr
   * @param[in] plain_num uint32_t The plaintext number
   * @return Expr The result of the greater-than operation. The slot is stored as a ciphertext state of 0/1.
   */
  friend Expr operator>(const Expr &lhs, uint32_t plain_num);

  /**
   * @brief Overloaded operator <= for ciphertext-ciphertext comparison
   * @details The operands, lhs and rhs, need to be expressions that have already undergone base decomposition.
   * @param[in] lhs Expr
   * @param[in] rhs Expr
   * @return Expr The result of the smaller-than-or-equal operation. The slot is stored as a ciphertext state of 0/1.
   */
  friend Expr operator<=(const Expr &lhs, const Expr &rhs);
  /**
   * @brief Overloaded operator <= for ciphertext-plaintext comparison
   * @details The operands, lhs and rhs, need to be expressions that have already undergone base decomposition.
   * @param[in] lhs Expr
   * @param[in] plain_num uint32_t The plaintext number
   * @return Expr The result of the smaller-than-or-equal operation. The slot is stored as a ciphertext state of 0/1.
   */
  friend Expr operator<=(const Expr &lhs, uint32_t plain_num);

  /// Smaller-than
  /**
   * @brief Overloaded operator < for ciphertext-ciphertext comparison
   * @details The operands, lhs and rhs, need to be expressions that have already undergone base decomposition.
   * @param[in] lhs Expr
   * @param[in] rhs Expr
   * @return Expr The result of the smaller-than operation. The slot is stored as a ciphertext state of 0/1.
   */
  friend Expr operator<(const Expr &lhs, const Expr &rhs);
  /**
   * @brief Overloaded operator < for ciphertext-plaintext comparison
   * @details The operands, lhs and rhs, need to be expressions that have already undergone base decomposition.
   * @param[in] lhs Expr
   * @param[in] plain_num uint32_t The plaintext number
   * @return Expr The result of the smaller-than operation. The slot is stored as a ciphertext state of 0/1.
   */
  friend Expr operator<(const Expr &lhs, uint32_t plain_num);

  /**
   * @brief Overloaded logical AND operator (&&) for ciphertext-plaintext comparison
   * @details The operands, lhs and rhs, need to be expressions that have already undergone base decomposition.
   * @param[in] lhs Expr
   * @param[in] rhs Expr
   * @return Expr The result of the logical AND operation. The slot is stored as a ciphertext state of 0/1.
   */
  friend Expr operator&&(const Expr &lhs, const Expr &rhs);

  /**
   * @brief Overloaded logical OR operator (||) for ciphertext-plaintext comparison
   * @details The operands, lhs and rhs, need to be expressions that have already undergone base decomposition.
   * @param[in] lhs Expr
   * @param[in] rhs Expr
   * @return Expr The result of the logical OR operation. The slot is stored as a ciphertext state of 0/1.
   */
  friend Expr operator||(const Expr &lhs, const Expr &rhs);

  /**
   * @brief Overloaded addition(+) operator as a friend function
   * @details  Operands can be Expr&, vector<double>, vector<int64_t>, vector<int32_t>, double, int64_t, int32_t, and other data types.
   * @todo Try to convert this to a template function.
   */
  friend Expr operator+(const Expr &lhs, const Expr &rhs);
  friend Expr operator+(const Expr &lhs, double lf);
  friend Expr operator+(double lf, const Expr &rhs);
  friend Expr operator+(const Expr &lhs, int32_t lf);
  friend Expr operator+(int32_t lf, const Expr &rhs);
  friend Expr operator+(const Expr &lhs, int64_t lf);
  friend Expr operator+(int64_t lf, const Expr &rhs);
  friend Expr operator+(const Expr &lhs, const vector<double> &rhs);
  friend Expr operator+(const vector<double> &lhs, const Expr &rhs);
  friend Expr operator+(const Expr &lhs, const vector<int64_t> &rhs);
  friend Expr operator+(const vector<int64_t> &lhs, const Expr &rhs);
  friend Expr operator+(const Expr &lhs, const vector<int32_t> &rhs);
  friend Expr operator+(const vector<int32_t> &lhs, const Expr &rhs);

  /**
   * @brief Overloaded subtraction(-) operator as a friend function
   * @details Operands can be Expr&, vector<double>, vector<int64_t>, vector<int32_t>, double, int64_t, int32_t, and other data types.
   * @todo Try to convert this to a template function.
   */
  friend Expr operator-(const Expr &lhs, const Expr &rhs);
  friend Expr operator-(const Expr &lhs, double lf);
  friend Expr operator-(double lf, const Expr &rhs);
  friend Expr operator-(const Expr &lhs, int32_t lf);
  friend Expr operator-(const Expr &lhs, int64_t lf);
  friend Expr operator-(int32_t lf, const Expr &rhs);
  friend Expr operator-(int64_t lf, const Expr &rhs);
  friend Expr operator-(const Expr &lhs, const vector<double> &rhs);
  friend Expr operator-(const vector<double> &lhs, const Expr &rhs);
  friend Expr operator-(const Expr &lhs, const vector<int64_t> &rhs);
  friend Expr operator-(const Expr &lhs, const vector<int32_t> &rhs);
  friend Expr operator-(const vector<int64_t> &lhs, const Expr &rhs);
  friend Expr operator-(const vector<int32_t> &lhs, const Expr &rhs);

  /**
   * @brief Overloaded multiplication(*) operator as a friend function
   * @details Operands can be Expr&, vector<double>, vector<int64_t>, vector<int32_t>, double, int64_t, int32_t, and other data types.
   * @todo Try to convert this to a template function.
   */
  friend Expr operator*(const Expr &lhs, const Expr &rhs);
  friend Expr operator*(const Expr &lhs, double lf);
  friend Expr operator*(double lf, const Expr &rhs);
  friend Expr operator*(const Expr &lhs, int32_t lf);
  friend Expr operator*(int32_t lf, const Expr &rhs);
  friend Expr operator*(const Expr &lhs, int64_t lf);
  friend Expr operator*(int64_t lf, const Expr &rhs);
  friend Expr operator*(const Expr &lhs, const vector<double> &rhs);
  friend Expr operator*(const vector<double> &lhs, const Expr &rhs);
  friend Expr operator*(const Expr &lhs, const vector<int64_t> &rhs);
  friend Expr operator*(const vector<int64_t> &lhs, const Expr &rhs);
  friend Expr operator*(const Expr &lhs, const vector<int32_t> &rhs);
  friend Expr operator*(const vector<int32_t> &lhs, const Expr &rhs);

  /**
   * @brief Overloaded internal left rotation operator as a friend function
   * @details It rotates the elements of the input vector by the specified index.
   * @param[in] lhs const Expr&
   * @param[in] k  uint32_t Number of positions to rotate left
   */
  friend Expr operator<<(const Expr &lhs, uint32_t k);
  /**
   * @brief Overloaded internal right rotation operator as a friend function
   * @details It rotates the elements of the input vector by the specified index.
   * @param[in] lhs const  Expr&
   * @param[in] k  uint32_t Number of positions to rotate right
   */
  friend Expr operator>>(const Expr &lhs, uint32_t k);

  /**
   * @brief Overloaded division(/) operator as a friend function
   * @details Concrete division, supporting only uint8_t type division.
   * @param[in] lhs const uint8_t Numerator in plaintext
   * @param[in] rhs const Expr& Denominator in ciphertext expression
   */
  friend Expr operator/(uint8_t lhs, const Expr &rhs);
  /**
   * @brief Overloaded division(/) operator as a friend function
   * @details Concrete division, supporting only uint8_t type division.
   * @param[in] lhs  const Expr& Numerator in ciphertext expression
   * @param[in] rhs  const uint8_t Denominator in plaintext
   */
  friend Expr operator/(const Expr &lhs, uint8_t rhs);

  /**
   * @brief Overloaded division(/) operator as a friend function
   * @details Supports CKKS algorithm-based simulation of division.
   * @param[in] lhs  const Expr& Numerator in ciphertext expression
   * @param[in] rhs const Expr& Denominator in ciphertext expression
   */
  friend Expr operator/(const Expr &lhs, const Expr &rhs);
  /**
   * @brief Overloaded division(/) operator as a friend function
   * @details Supports CKKS algorithm-based simulation of division.
   * @param[in] lhs const Expr& Numerator in ciphertext expression
   * @param[in] rhs const double Denominator in plaintext double
   */
  friend Expr operator/(const Expr &lhs, double rhs);
  /**
   * @brief Overloaded division(/) operator as a friend function
   * @details Supports CKKS algorithm-based simulation of division.
   * @param[in] lhs  const double Numerator in plaintext double
   * @param[in] rhs  const Expr& Denominator in ciphertext expression
   */
  friend Expr operator/(double lhs, const Expr &rhs);
  /**
   * @brief Overloaded division(/) operator as a friend function
   * @details Supports CKKS algorithm-based simulation of division.
   * @param[in] lhs const vector<double>& Numerator in plaintext vector
   * @param[in] rhs const Expr& Denominator in ciphertext expression
   */
  friend Expr operator/(const vector<double> &lhs, const Expr &rhs);

  /**
   * @brief FFT-transformed division operator as a friend function
   * @details Returns the result after FFT transformation.
   * @param[in] lhs  const ComplexExpr &
   * @param[in] rhs  const ComplexExpr &
   */
  friend ComplexExpr operator/(const ComplexExpr &lhs, const ComplexExpr &rhs);

  /**
   * @brief Construct a query for the specified column that satisfies the given conditions
   * @details Use ckks in conjunction with FFT for preprocessing the query data.
   * @param[in] lhs  const Expr & Column to be queried, preprocessed under plaintext FFT
   * @param[in] rhs  const Expr & Query condition
   * @return Expr with the FFT ciphertext data of the corresponding column
   */
  friend Expr QueryRow(const Expr &lhs, const Expr &rhs);
  /**
   * @brief Construct a query for the count satisfying the given conditions
   * @details
   * @param[in] lhs  const Expr & Query condition
   * @return Expr with the count in ciphertext, located in slot 0
   */
  friend Expr QueryCnt(const Expr &lhs);
  /**
   * @brief Construct a query for the sum of the specified column satisfying the given conditions
   * @details Use ckks in conjunction with FFT for preprocessing the query data.
   * @param[in] lhs  const Expr & Column to be queried, preprocessed under plaintext FFT
   * @param[in] rhs  const Expr & Query condition
   * @return Expr with the FFT ciphertext data of the sum of the columns satisfying the conditions, the sum result is in the 32nd slot set
   */
  friend Expr QuerySum(const Expr &lhs, const Expr &rhs);
  // friend Expr QueryAvg(const Expr &lhs, const Expr &rhs);

  /**
   * @brief Construct a comparison expression helper function
   * @details Plain data comparison requires base decomposition before encryption
   * @param[in] lhs  const Expr & Left operand of the comparison
   * @param[in] rhs  const Expr & Right operand of the comparison
   * @param[in] type CMP_TYPE Type of the comparison (equal, less than, etc.)
   * @return Comparison operation Expr
   */
  friend Expr CmpOpHelper(const Expr &lhs, const Expr &rhs, CMP_TYPE type);
  friend Expr SumCntHelper(const Expr &lhs);

 public:
  // std::shared_ptr<Dag> m_dag;  // dag
  Dag *m_dag;
  NodePtr m_nodeptr;

 private:
  // need
  bool m_plaintext;
  int m_multdepth = 0;
};

/**
 * @brief Generate comparison results with 3 as the base for comparison operations
 * @details Comparison of plaintext data requires base decomposition before encryption
 * @param[in] input_expr  Input expression
 * @param[out] lt_result  Expression result for less than
 * @param[out] eq_result  Expression result for equal
 */
void getCmpExprP3(const Expr &input_expr, Expr &lt_result, Expr &eq_result);

/**
 * @brief Generate comparison results with 7 as the base for comparison operations
 * @details Comparison of plaintext data requires base decomposition before encryption
 * @param[in] input_expr  Input expression
 * @param[out] lt_result  Expression result for less than
 * @param[out] eq_result  Expression result for equal
 */
void getCmpExprP7(const Expr &input_expr, Expr &lt_result, Expr &eq_result);

};  // namespace iyfc
