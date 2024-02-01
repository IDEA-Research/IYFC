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

/* Check whether the various data types + operator overloading in the expr file are correct*/
namespace iyfctest {
/**
 * @brief Test suite for double data type and operator overloads.
 *        ExprTestDouble suite with Expr objects having double as the second template parameter.
 */
class ExprTestDouble : public ::testing::Test {
protected:

    DagPtr dag;
    Expr x;
    Valuation inputs;

    // Set up the environment before each test case.
    void SetUp() override {
        dag = initDag("test", 2);
        x = setInputName(dag, "x");
        inputs["x"] = 2.0;
    }

    void TearDown() override {
        // Tear down the environment after each test case.
        releaseDag(dag);
    }

};


// Test += Addition Assignment
// 1. Expr += Expr (both double)
TEST_F(ExprTestDouble, AddAssignTest) {
    Expr y = setInputName(dag, "y");
    inputs["y"] = 3.0;
    x += y;
    Valuation output = execute(inputs, dag, x);

    // // Convert output to string and print
    // stringstream ss;
    // ss << "output: ";
    // for (const auto& item : output) {
    //     ss << item.first << ": " << get<vector<double>>(item.second)[0] << ", ";
    // }
    // ss << std::endl;
    // cout << ss.str();
    // // Continue to print the value of get
    // cout << "get output: " << get<vector<double>>(output["test_out"])[0] << endl;

    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], 5, 0.001);
}
// 2. Expr(double) += double
TEST_F(ExprTestDouble, AddAssignTestWithDouble){
    x += 3.0;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], 5.0, 0.001);
}
// 3. Expr(double) += int
TEST_F(ExprTestDouble, AddAssignTestWithInt){
    x += 3;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<int64_t>>(output["test_out"])[0], 5, 0.001);
}
// 4. Expr(double) += vector<double>
TEST_F(ExprTestDouble, AddAssignTestWithVectorDouble){
    x += vector<double>{3.0, 4.0};
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>{5.0, 6.0}, 0.001);
}
// 5. Expr(double) += vector<int64_t>
TEST_F(ExprTestDouble, AddAssignTestWithVectorInt){
    x += vector<int64_t>{3, 4};
    Valuation output = execute(inputs, dag, x);
    check_result<int64_t>(output, vector<int64_t>{5, 6}, 0.001);
}


// Test -= Subtraction Assignment
// 1. Expr -= Expr (both double)
TEST_F(ExprTestDouble, SubAssignTest) {
    Expr y = setInputName(dag, "y");
    inputs["y"] = 3.0;
    x -= y;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], -1, 0.001);
}
// 2. Expr(double) -= double
TEST_F(ExprTestDouble, SubAssignTestWithDouble){
    x -= 3.0;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], -1, 0.001);
}
// 3. Expr(double) -= int
TEST_F(ExprTestDouble, SubAssignTestWithInt){
    x -= 3;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<int64_t>>(output["test_out"])[0], -1, 0.001);
}
// 4. Expr(double) -= vector<double>
TEST_F(ExprTestDouble, SubAssignTestWithVectorDouble){
    x -= vector<double>{3.0, 4.0};
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>{-1, -2}, 0.001);
}
// 5. Expr(double) -= vector<int64_t>
TEST_F(ExprTestDouble, SubAssignTestWithVectorInt){
    x -= vector<int64_t>{3, 4};
    Valuation output = execute(inputs, dag, x);
    check_result<int64_t>(output, vector<int64_t>{-1, -2}, 0.001);
}


// Test *= Multiplication Assignment
// 1. Expr *= Expr (both double)
TEST_F(ExprTestDouble, MulAssignTest) {
    Expr y = setInputName(dag, "y");
    inputs["y"] = 3.0;
    x *= y;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], 6, 0.001);
}
// 2. Expr(double) *= double
TEST_F(ExprTestDouble, MulAssignTestWithDouble){
    x *= 3.0;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], 6, 0.001);
}
// 3. Expr(double) *= int
TEST_F(ExprTestDouble, MulAssignTestWithInt){
    x *= 3;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<int64_t>>(output["test_out"])[0], 6, 0.001);
}
// 4. Expr(double) *= vector<double>
TEST_F(ExprTestDouble, MulAssignTestWithVectorDouble){
    // x *= vector<double>{3.0, 4.0};
    // x.operator*=(vector<double>{3.0, 4.0});
    Expr y = x * vector<double>{3.0, 4.0};
    Valuation output = execute(inputs, dag, y);
    check_result<double>(output, vector<double>{6, 8}, 0.001);
}
// 5. Expr(double) *= vector<int64_t>
TEST_F(ExprTestDouble, MulAssignTestWithVectorInt){
    // x *= vector<int64_t>{3, 4};
    Expr y = x * vector<int64_t>{3, 4};
    Valuation output = execute(inputs, dag, y);
    check_result<int64_t>(output, vector<int64_t>{6, 8}, 0.001);
}


// Test ++ Increment operator
TEST_F(ExprTestDouble, IncrementTest){
    x++;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], 3, 0.001);
}

// Test -- Decrement operator
TEST_F(ExprTestDouble, DecrementTest){
    x--;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], 1, 0.001);
}


/**
 * @brief Test suite for int data type and operator overloads.
 *        ExprTestInt suite with Expr objects having int as the second template parameter.
 */
class ExprTestInt : public ::testing::Test {
protected:
    // Set up the environment before each test case.
    void SetUp() override {
        dag = initDag("test", 2);
        x = setInputName(dag, "x");
        inputs["x"] = static_cast<int64_t>(2);
    }

    void TearDown() override {
        // Tear down the environment after each test case.
        releaseDag(dag);
    }

    DagPtr dag;
    Expr x;
    Valuation inputs;
};

// Test += Addition Assignment
// 1. Expr += Expr (both int)
TEST_F(ExprTestInt, AddAssignTest) {
    Expr y = setInputName(dag, "y");
    inputs["y"] = static_cast<int64_t>(3);
    x += y;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], 5, 0.001);
}
// 2. Expr(int) += int
TEST_F(ExprTestInt, AddAssignTestWithInt) {
    x += 3;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<int64_t>>(output["test_out"])[0], 5, 0.001);
}
// 3. Expr(int) += double
TEST_F(ExprTestInt, AddAssignTestWithDouble){
    x += 3.0;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], 5, 0.001);
}
// 4. Expr(int) += vector<double>
TEST_F(ExprTestInt, AddAssignTestWithVectorDouble){
    x += vector<double>{3.0, 4.0};
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>{5.0, 6.0}, 0.001);
}
// 5. Expr(int) += vector<int64_t>
TEST_F(ExprTestInt, AddAssignTestWithVectorInt){
    x += vector<int64_t>{3, 4};
    Valuation output = execute(inputs, dag, x);
    check_result<int64_t>(output, vector<int64_t>{5, 6}, 0.001);
}


// Test -= Subtraction Assignment
// 1. Expr -= Expr (both int)
TEST_F(ExprTestInt, SubAssignTest) {
    Expr y = setInputName(dag, "y");
    inputs["y"] = static_cast<int64_t>(3);
    x -= y;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], -1, 0.001);
}
// 2. Expr(int) -= int
TEST_F(ExprTestInt, SubAssignTestWithInt) {
    x -= 3;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<int64_t>>(output["test_out"])[0], -1, 0.001);
}
// 3. Expr(int) -= double
TEST_F(ExprTestInt, SubAssignTestWithDouble){
    x -= 3.0;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], -1, 0.001);
}
// 4. Expr(int) -= vector<double>
TEST_F(ExprTestInt, SubAssignTestWithVectorDouble){
    x -= vector<double>{3.0, 4.0};
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>{-1, -2}, 0.001);
}
// 5. Expr(int) -= vector<int64_t>
TEST_F(ExprTestInt, SubAssignTestWithVectorInt){
    x -= vector<int64_t>{3, 4};
    Valuation output = execute(inputs, dag, x);
    check_result<int64_t>(output, vector<int64_t>{-1, -2}, 0.001);
}

// Test *= Multiplication Assignment
// 1. Expr *= Expr (both int)
TEST_F(ExprTestInt, MulAssignTest) {
    Expr y = setInputName(dag, "y");
    inputs["y"] = static_cast<int64_t>(3);
    x *= y;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], 6, 0.001);
}
// 2. Expr(int) *= int
TEST_F(ExprTestInt, MulAssignTestWithInt) {
    x *= 3;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<int64_t>>(output["test_out"])[0], 6, 0.001);
}
// 3. Expr(int) *= double
TEST_F(ExprTestInt, MulAssignTestWithDouble){
    x *= 3.0;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], 6, 0.001);
}
// 4. Expr(int) *= vector<double>    Q2
TEST_F(ExprTestInt, MulAssignTestWithVectorDouble){
    // x *= vector<double>{3.0, 4.0};
    Expr y = x * vector<double>{3.0, 4.0};
    Valuation output = execute(inputs, dag, y);
    check_result<double>(output, vector<double>{6, 8}, 0.001);
}
// 5. Expr(int) *= vector<int64_t>    Q2
TEST_F(ExprTestInt, MulAssignTestWithVectorInt){
    // x *= vector<int64_t>{3, 4};
    Expr y = x * vector<int64_t>{3, 4};
    Valuation output = execute(inputs, dag, y);
    check_result<int64_t>(output, vector<int64_t>{6, 8}, 0.001);
}

// Test ++ Increment operator
TEST_F(ExprTestInt, IncrementTest){
    x++;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], 3, 0.001);
}

// Test -- Decrement operator
TEST_F(ExprTestInt, DecrementTest){
    x--;
    Valuation output = execute(inputs, dag, x);
    EXPECT_NEAR(get<vector<double>>(output["test_out"])[0], 1, 0.001);
}


/**
 * @brief Test suite for vector<double> data type.
 *        ExprTestVectorDouble suite with Expr objects having vector<double> as the second template parameter.
 */
class ExprTestVectorDouble : public ::testing::Test {
protected:
    // Set up the environment before each test case.
    void SetUp() override {
        dag = initDag("test", 1024);
        x = setInputName(dag, "x");
        inputs["x"] = vector<double>(1024, 2.0);
    }

    void TearDown() override {
        // Tear down the environment after each test case.
        releaseDag(dag);
    }

    DagPtr dag;
    Expr x;
    Valuation inputs;
};

// Test += Addition Assignment
// 1. Expr += Expr (both vector<double>)
TEST_F(ExprTestVectorDouble, AddAssignTest) {
    Expr y = setInputName(dag, "y");
    inputs["y"] = vector<double>(1024, 3.0);
    cout << getVecSize(dag);
    x += y;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, 5.0), 0.001);
}
// 2. Expr(vector<double>) += double
TEST_F(ExprTestVectorDouble, AddAssignTestWithDouble){
    x += 3.0;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, 5.0), 0.001);
}
// 3. Expr(vector<double>) += int
TEST_F(ExprTestVectorDouble, AddAssignTestWithInt){
        x += 3;
        Valuation output = execute(inputs, dag, x);
        check_result<int64_t>(output, vector<int64_t>(1024, 5), 0.001);
}
// 4. Expr(vector<double>) += vector<double>
TEST_F(ExprTestVectorDouble, AddAssignTestWithVectorDouble){
    x += vector<double>(1024, 3.0);
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, 5.0), 0.001);
}
// 5. Expr(vector<double>) += vector<int64_t>
TEST_F(ExprTestVectorDouble, AddAssignTestWithVectorInt){
    x += vector<int64_t>(1024, 3);
    Valuation output = execute(inputs, dag, x);
    check_result<int64_t>(output, vector<int64_t>(1024, 5.0), 0.001);
}


// Test -= Subtraction Assignment
// 1. Expr -= Expr (both vector<double>)
TEST_F(ExprTestVectorDouble, SubAssignTest) {
    Expr y = setInputName(dag, "y");
    inputs["y"] = vector<double>(1024, 3.0);
    cout << getVecSize(dag);
    x -= y;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, -1), 0.001);
}
// 2. Expr(vector<double>) -= double
TEST_F(ExprTestVectorDouble, SubAssignTestWithDouble){
    x -= 3.0;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, -1), 0.001);
}
// 3. Expr(vector<double>) -= int   err: Floating point exception
// TEST_F(ExprTestVectorDouble, SubAssignTestWithInt){
//     x -= 3;
//     Valuation output = execute(inputs, dag, x);
//     check_result<double>(output, vector<double>(1024, -1), 0.001);
// }
// 4. Expr(vector<double>) -= vector<double>
TEST_F(ExprTestVectorDouble, SubAssignTestWithVectorDouble){
    x -= vector<double>(1024, 3.0);
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, -1), 0.001);
}
// 5. Expr(vector<double>) -= vector<int64_t>
TEST_F(ExprTestVectorDouble, SubAssignTestWithVectorInt){
    x -= vector<int64_t>(1024, 3);
    Valuation output = execute(inputs, dag, x);
    check_result<int64_t>(output, vector<int64_t>(1024, -1), 0.001);
}


// Test *= Multiplication Assignment
// 1. Expr *= Expr (both vector<double>)
TEST_F(ExprTestVectorDouble, MulAssignTest) {
    Expr y = setInputName(dag, "y");
    inputs["y"] = vector<double>(1024, 3.0);
    cout << getVecSize(dag);
    x *= y;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, 6), 0.001);
}
// 2. Expr(vector<double>) *= double
TEST_F(ExprTestVectorDouble, MulAssignTestWithDouble){
    x *= 3.0;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, 6), 0.001);
}
// 3. Expr(vector<double>) *= int
TEST_F(ExprTestVectorDouble, MulAssignTestWithInt){
    x *= 3;
    Valuation output = execute(inputs, dag, x);
    check_result<int64_t>(output, vector<int64_t>(1024, 6), 0.001);
}
// 4. Expr(vector<double>) *= vector<double>   Q2
TEST_F(ExprTestVectorDouble, MulAssignTestWithVectorDouble){
    // x *= vector<double>(1024, 3.0);
    Expr y = x * vector<double>(1024, 3.0);
    Valuation output = execute(inputs, dag, y);
    check_result<double>(output, vector<double>(1024, 6), 0.001);
}
// 5. Expr(vector<double>) *= vector<int64_t>   Q2
TEST_F(ExprTestVectorDouble, MulAssignTestWithVectorInt){
    // x *= vector<int64_t>(1024, 3);
    Expr y = x * vector<int64_t>(1024, 3);
    Valuation output = execute(inputs, dag, y);
    check_result<int64_t>(output, vector<int64_t>(1024, 6), 0.001);
}


// Test ++ Increment operator
TEST_F(ExprTestVectorDouble, IncrementTest){
    x++;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, 3), 0.001);
}

// Test -- Decrement operator
TEST_F(ExprTestVectorDouble, DecrementTest){
    x--;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, 1), 0.001);
}


/**
 * @brief Test suite for vector<int64_t> data type.
 *        ExprTestVectorInt suite with Expr objects having vector<int64_t> as the second template parameter.
 */
class ExprTestVectorInt : public ::testing::Test {
protected:
    // Set up the environment before each test case.
    void SetUp() override {
        dag = initDag("test", 1024);
        x = setInputName(dag, "x");
        inputs["x"] = vector<int64_t>(1024, 2);
    }

    void TearDown() override {
        // Tear down the environment after each test case.
        releaseDag(dag);
    }

    DagPtr dag;
    Expr x;
    Valuation inputs;
};

// Test += Addition Assignment
// 1. Expr += Expr (both vector<int64_t>)
TEST_F(ExprTestVectorInt, AddAssignTest) {
    Expr y = setInputName(dag, "y");
    inputs["y"] = vector<int64_t>(1024, 3);
    x += y;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, 5), 0.001);
}
// 2. Expr<vector<int64_t> += int
TEST_F(ExprTestVectorInt, AddAssignTestWithInt){
    x += 3;
    Valuation output = execute(inputs, dag, x);
    check_result<int64_t>(output, vector<int64_t>(1024, 5), 0.001);
}
// 3. Expr(vector<int64_t>) += double
TEST_F(ExprTestVectorInt, AddAssignTestWithDouble){
    x += 3.0;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, 5), 0.001);
}
// 4. Expr(vector<int64_t>) += vector<double>
TEST_F(ExprTestVectorInt, AddAssignTestWithVectorDouble){
    x += vector<double>(1024, 3);
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, 5), 0.001);
}
// 5. Expr(vector<int64_t>) += vector<int64_t>
TEST_F(ExprTestVectorInt, AddAssignTestWithVectorInt){
    x += vector<int64_t>(1024, 3);
    Valuation output = execute(inputs, dag, x);
    check_result<int64_t>(output, vector<int64_t>(1024, 5), 0.001);
}


// Test -= Subtraction Assignment
// 1. Expr -= Expr (both vector<int64_t>)
TEST_F(ExprTestVectorInt, SubAssignTest) {
    Expr y = setInputName(dag, "y");
    inputs["y"] = vector<int64_t>(1024, 3);
    x -= y;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, -1), 0.001);
}
// 2. Expr<vector<int64_t> -= int
TEST_F(ExprTestVectorInt, SubAssignTestWithInt){
    x -= 3;
    Valuation output = execute(inputs, dag, x);
    check_result<int64_t>(output, vector<int64_t>(1024, -1), 0.001);
}
// 3. Expr(vector<int64_t>) -= double
TEST_F(ExprTestVectorInt, SubAssignTestWithDouble){
    x -= 3.0;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, -1), 0.001);
}
// 4. Expr(vector<int64_t>) -= vector<double>
TEST_F(ExprTestVectorInt, SubAssignTestWithVectorDouble){
    x -= vector<double>(1024, 3);
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, -1), 0.001);
}
// 5. Expr(vector<int64_t>) -= vector<int64_t>
TEST_F(ExprTestVectorInt, SubAssignTestWithVectorInt){
    x -= vector<int64_t>(1024, 3);
    Valuation output = execute(inputs, dag, x);
    check_result<int64_t>(output, vector<int64_t>(1024, -1), 0.001);
}


// Test *= Multiplication Assignment
// 1. Expr *= Expr (both vector<int64_t>)
TEST_F(ExprTestVectorInt, MulAssignTest) {
    Expr y = setInputName(dag, "y");
    inputs["y"] = vector<int64_t>(1024, 3);
    x *= y;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, 6), 0.001);
}
// 2. Expr<vector<int64_t> *= int
TEST_F(ExprTestVectorInt, MulAssignTestWithInt){
    x *= 3;
    Valuation output = execute(inputs, dag, x);
    check_result<int64_t>(output, vector<int64_t>(1024, 6), 0.001);
}
// 3. Expr(vector<int64_t>) *= double
TEST_F(ExprTestVectorInt, MulAssignTestWithDouble){
    x *= 3.0;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, 6), 0.001);
}
// 4. Expr(vector<int64_t>) *= vector<double>    Q2
TEST_F(ExprTestVectorInt, MulAssignTestWithVectorDouble){
    // x *= vector<double>(1024, 3);
    Expr y = x * vector<double>(1024, 3);
    Valuation output = execute(inputs, dag, y);
    check_result<double>(output, vector<double>(1024, 6), 0.001);
}
// 5. Expr(vector<int64_t>) *= vector<int64_t>    Q2
TEST_F(ExprTestVectorInt, MulAssignTestWithVectorInt){
    // x *= vector<int64_t>(1024, 3);
    Expr y = x * vector<int64_t>(1024, 3);
    Valuation output = execute(inputs, dag, y);
    check_result<int64_t>(output, vector<int64_t>(1024, 6), 0.001);
}


// Test ++ Increment operator
TEST_F(ExprTestVectorInt, IncrementTest){
    x++;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, 3), 0.001);
}

// Test -- Decrement operator
TEST_F(ExprTestVectorInt, DecrementTest){
    x--;
    Valuation output = execute(inputs, dag, x);
    check_result<double>(output, vector<double>(1024, 1), 0.001);
}


} // namespace iyfctest
