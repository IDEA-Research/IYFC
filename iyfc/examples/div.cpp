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

// Mathematical constant (PI), used for DFT
#define PI 3.141592653589793

// =================================== Subfunction Definitions
// ======================================== DFT Transformation
class DFT {
 public:
  static void dft(vector<complex<double>> &vec_in,
                  vector<complex<double>> &vec_out, int &N, bool inverse);

 private:
  static vector<complex<double>> temp_vec;
};

vector<std::complex<double>> DFT::temp_vec;

void DFT::dft(vector<complex<double>> &vec_in, vector<complex<double>> &vec_out,
              int &N, bool inverse)  // For inverse transformation, set inverse to true;
                                     // For regular transformation, set inverse to false.
{
  temp_vec.assign(vec_in.begin(), vec_in.end());  // temp_vec is a copy of vec

  complex<double> temp;
  for (int m = 0; m < N; ++m) {
    temp = 0;
    for (int n = 0; n < N; ++n) {
      temp += temp_vec[n] * polar(1.0, (inverse ? 1 : -1) * 2 * PI * n * m /
                                           N);  // Derived from Euler's formula
    }
    vec_out.push_back(inverse ? temp / (double)N : temp);
  }
}

void example_div() {
  // ===================================================================
  // Data Preparation Phase: Determine dividend, divisor, and modulus,
  // and perform DFT transformation and encoding
  // ===================================================================
  srand((unsigned)time(NULL));
  int modulus_p = INT_MAX;  // 1<<8; // Modulus
  cout << "模数为：" << modulus_p << endl;
  // int num_to_be_divided = modulus_p / 3 + 20232; // Fixed dividend
  int num_to_be_divided = rand() % (modulus_p);  // Random dividend in the range (0, p)
  cout << "被除数为：" << num_to_be_divided << endl;
  // int num_origin = modulus_p / 2 + 18123; // Fixed divisor
  int num_origin = (rand() % (modulus_p / 2 - 1)) + modulus_p / 2 +
                   1;  // Random divisor in the range (p/2, p)
  cout << "除数为：" << num_origin << endl;
  int POLY = 1024;

  int num_divide = modulus_p - num_origin;

  // step 1.1 Modulus p
  // Convert to a reverse vector, for example, if p=123456,
  //  convert it to vec_num_to_be_divided = [6,5,4,3,2,1]
  stringstream sstr_p;
  sstr_p << modulus_p;
  string str_modulus_p = sstr_p.str();
  // cout << "string of modulus array: ";
  // for (auto & i : str_modulus_p) cout << i << " ";
  // cout << endl;

  // Modulus length
  int length_modulus = str_modulus_p.size();
  // cout << "length of modulus array: " << length_modulus << endl;

  // Convert string to vector
  vector<complex<double>> vec_modulus_p;
  for (char i : str_modulus_p) vec_modulus_p.emplace_back(i - '0');
  reverse(vec_modulus_p.begin(), vec_modulus_p.end());
  // cout << "modulus array：";
  // for (auto & i : vec_modulus_p) cout << i.real() << " ";
  // cout << endl;

  // step 1.2 Convert dividend to a reverse vector
  stringstream sstr_to_be_divided;

  sstr_to_be_divided << num_to_be_divided;
  string str_num_to_be_divided = sstr_to_be_divided.str();
  // cout << "string of dividend array: ";
  // for (auto & i : str_num_to_be_divided) cout << i << " ";
  // cout << endl;

  // Dividend length
  int length_to_be_divided = str_num_to_be_divided.size();
  // cout << "length of dividend array: " << length_to_be_divided << endl;

  // Convert string to vector
  vector<complex<double>> vec_num_to_be_divided;
  for (char i : str_num_to_be_divided)
    vec_num_to_be_divided.emplace_back(i - '0');
  reverse(vec_num_to_be_divided.begin(), vec_num_to_be_divided.end());
  // cout << "dividend array: ";
  // for (auto & i : vec_num_to_be_divided) cout << i.real() << " ";
  // cout << endl;

  // step 1.3 Convert p minus divisor to a reverse vector
  stringstream sstr_divide;
  sstr_divide << num_divide;
  string str_num_divide = sstr_divide.str();
  // cout << "string of p minus divisor array: ";
  // for (auto & i : str_num_divide) cout << i << " ";
  // cout << endl;

  // Divisor length
  int length_divide = str_num_divide.size();
  // cout << "length of p minus divisor array: " << length_divide << endl;

  // Convert string to vector
  vector<complex<double>> vec_num_divide;
  for (char i : str_num_divide) vec_num_divide.emplace_back(i - '0');
  reverse(vec_num_divide.begin(), vec_num_divide.end());
  // cout << "p minus divisor array: ";
  // for (auto & i : vec_num_divide) cout << i.real() << " ";
  // cout << endl;

  // step 2 Add zeros to dividend, divisor, and modulus arrays, making their lengths 8*length_modulus-7
  int length_DFT = 8 * length_modulus - 7;
  int padding_length;
  padding_length = length_DFT - length_modulus;  // Add zeros to the modulus
  for (int i = 0; i < padding_length; i++) vec_modulus_p.emplace_back(0);
  padding_length = length_DFT - length_to_be_divided;
  for (int i = 0; i < padding_length; i++)
    vec_num_to_be_divided.emplace_back(0);  // Add zeros to the dividend
  padding_length = length_DFT - length_divide;
  for (int i = 0; i < padding_length; i++)
    vec_num_divide.emplace_back(0);  // Add zeros to the divisor
  // cout << vec_modulus_p.size() << " " << vec_num_to_be_divided.size() << " "
  // << vec_num_divide.size() << endl;

  // step 3-1 Perform DFT transformation on the arrays with added zeros, 
  // and store the results in DFT_p, DFT_to_be_divided, and DFT_divide
  vector<complex<double>> DFT_p;
  DFT::dft(vec_modulus_p, DFT_p, length_DFT, false);  // Modulus DFT transformation
  // cout << "modulus DFT result: " << endl;
  // for (auto &item:DFT_p) cout << item << endl;
  //    for (auto &item:DFT_p) cout << item.real() << endl; // Modulo DFT transformation - real part
  //    for (auto &item:DFT_p) cout << item.imag() << endl; // Modulo DFT transformation - imaginary part

  vector<complex<double>> DFT_to_be_divided;
  DFT::dft(vec_num_to_be_divided, DFT_to_be_divided, length_DFT,
           false);  // Dividend DFT transformation
  // cout << "dividend DFT transformation result: " << endl;
  // for (auto &item:DFT_to_be_divided) cout << item << endl;
  //    for (auto &item:DFT_to_be_divided) cout << item.real() << endl; //
  //    dividend DFT transformation - real part for (auto &item:DFT_to_be_divided) cout << item.imag()
  //    << endl; // dividend DFT transformation - imaginary part

  vector<complex<double>> DFT_divide;
  DFT::dft(vec_num_divide, DFT_divide, length_DFT, false);  // Divisor DFT transformation
  // cout << "divisor DFT result: " << endl;
  // for (auto &item:DFT_divide) cout << item << endl;
  //    for (auto &item:DFT_divide) cout << item.real() << endl; //
  //    divisor DFT transformation - real part for (auto &item:DFT_divide) cout << item.imag() << endl;
  //    // divisor DFT transformation - imaginary part 

  // step 3-2 Test if the DFT results are correct by performing inverse DFT on them
  vector<complex<double>> IDFT_p;
  DFT::dft(DFT_p, IDFT_p, length_DFT, true);  // Modulus IDFT transformation
  // cout << "modulus IDFT result: " << endl;
  // for (auto &item:IDFT_p) cout << round(item.real()) << endl; //
  // modulus IDFT result

  vector<complex<double>> IDFT_to_be_divided;
  DFT::dft(DFT_to_be_divided, IDFT_to_be_divided, length_DFT,
           true);  // Dividend IDFT transformation
  // cout << "dividend IDFT transformation result: " << endl;
  // for (auto &item:IDFT_to_be_divided) cout << round(item.real()) << endl; //
  // dividend DFT transformation result

  vector<complex<double>> IDFT_divide;
  DFT::dft(DFT_divide, IDFT_divide, length_DFT, true);  // Divisor IDFT transformation
  // cout << "divisor IDFT result: "<< endl;
  // for (auto &item:IDFT_divide) cout << round(item.real()) << endl; //
  // divisor IDFT transformation result

  // =============================================================
  // Homomorphic Program Definition Phase: Build computation DAG,
  // set inputs, construct computation logic
  // =====================  step 0 Build comparison computation DAG
  DagPtr dag = initDag("fft-divison");

  // step 2
  // Set inputs: 6 inputs, representing the real and imaginary parts of modulus, dividend and divisor.
  // If complex encoding is supported later, this can be simplified to 3 inputs.
  Expr p_real = setInputName(dag, "pr");                // Modulus real part
  Expr p_imag = setInputName(dag, "pi");                // Modulus imaginary part
  Expr to_be_divided_real = setInputName(dag, "dedr");  // dividend real part
  Expr to_be_divided_imag = setInputName(dag, "dedi");  // dividend imaginary part
  Expr divide_real = setInputName(dag, "dr");           // divisor real part
  Expr divide_imag = setInputName(dag, "di");           // divisor imaginary part

  // step 3 Construct computation logic
  // Note 1: What is the purpose of setting {1.0} for nodes? How is it different from {1.0,2.0} -> {1.0,2.0,3.0}?

  // Declare the real and imaginary parts of operands
  vector<double> init_num = {1.0};
  Expr operand_a_real = Expr(dag, init_num);
  Expr operand_a_imag = Expr(dag, init_num);
  Expr operand_b_real = Expr(dag, init_num);
  Expr operand_b_imag = Expr(dag, init_num);

  // step 3-1 Compute p+x
  operand_a_real = p_real;
  operand_a_imag = p_imag;
  operand_b_real = divide_real;
  operand_b_imag = divide_imag;
  Expr p_x_real = Expr(dag, init_num);
  Expr p_x_imag = Expr(dag, init_num);
  p_x_real = operand_a_real + operand_b_real;
  p_x_imag = operand_a_imag + operand_b_imag;

  // step 3-2 Compute p^2 + x^2
  // Compute p^2 = p * p
  operand_a_real = p_real;
  operand_a_imag = p_imag;
  operand_b_real = p_real;
  operand_b_imag = p_imag;
  Expr p_2_real = Expr(dag, init_num);
  Expr p_2_imag = Expr(dag, init_num);
  p_2_real = operand_a_real * operand_b_real - operand_a_imag * operand_b_imag;
  p_2_imag = operand_a_real * operand_b_imag + operand_a_imag * operand_b_real;

  // Compute x^2 = x * x
  operand_a_real = divide_real;
  operand_a_imag = divide_imag;
  operand_b_real = divide_real;
  operand_b_imag = divide_imag;
  Expr x_2_real = Expr(dag, init_num);
  Expr x_2_imag = Expr(dag, init_num);
  x_2_real = operand_a_real * operand_b_real - operand_a_imag * operand_b_imag;
  x_2_imag = operand_a_real * operand_b_imag + operand_a_imag * operand_b_real;

  // Compute p^2 + x^2
  operand_a_real = p_2_real;
  operand_a_imag = p_2_imag;
  operand_b_real = x_2_real;
  operand_b_imag = x_2_imag;
  Expr p_2_x_2_real = Expr(dag, init_num);
  Expr p_2_x_2_imag = Expr(dag, init_num);
  p_2_x_2_real = operand_a_real + operand_b_real;
  p_2_x_2_imag = operand_a_imag + operand_b_imag;

  // step 3-3 Compute p^4 + x^4
  // Compute p^4 = p^2 * p^2
  operand_a_real = p_2_real;
  operand_a_imag = p_2_imag;
  operand_b_real = p_2_real;
  operand_b_imag = p_2_imag;
  Expr p_4_real = Expr(dag, init_num);
  Expr p_4_imag = Expr(dag, init_num);
  p_4_real = operand_a_real * operand_b_real - operand_a_imag * operand_b_imag;
  p_4_imag = operand_a_real * operand_b_imag + operand_a_imag * operand_b_real;

  // Compute x^4 = x^2 * x^2
  operand_a_real = x_2_real;
  operand_a_imag = x_2_imag;
  operand_b_real = x_2_real;
  operand_b_imag = x_2_imag;
  Expr x_4_real = Expr(dag, init_num);
  Expr x_4_imag = Expr(dag, init_num);
  x_4_real = operand_a_real * operand_b_real - operand_a_imag * operand_b_imag;
  x_4_imag = operand_a_real * operand_b_imag + operand_a_imag * operand_b_real;

  // Compute p^4 + x^4
  operand_a_real = p_4_real;
  operand_a_imag = p_4_imag;
  operand_b_real = x_4_real;
  operand_b_imag = x_4_imag;
  Expr p_4_x_4_real = Expr(dag, init_num);
  Expr p_4_x_4_imag = Expr(dag, init_num);
  p_4_x_4_real = operand_a_real + operand_b_real;
  p_4_x_4_imag = operand_a_imag + operand_b_imag;

  // step 3-4 Compute (p + x) * (p^2 + x^2)
  operand_a_real = p_x_real;
  operand_a_imag = p_x_imag;
  operand_b_real = p_2_x_2_real;
  operand_b_imag = p_2_x_2_imag;
  Expr p_x_p_2_x_2_real = Expr(dag, init_num);
  Expr p_x_p_2_x_2_imag = Expr(dag, init_num);
  p_x_p_2_x_2_real =
      operand_a_real * operand_b_real - operand_a_imag * operand_b_imag;
  p_x_p_2_x_2_imag =
      operand_a_real * operand_b_imag + operand_a_imag * operand_b_real;

  // step 3-5 Compute (p + x) * (p^2 + x^2) *(p^4 + x^4)
  operand_a_real = p_x_p_2_x_2_real;
  operand_a_imag = p_x_p_2_x_2_imag;
  operand_b_real = p_4_x_4_real;
  operand_b_imag = p_4_x_4_imag;
  Expr p_x_p_2_x_2_p_4_x_4_real = Expr(dag, init_num);
  Expr p_x_p_2_x_2_p_4_x_4_imag = Expr(dag, init_num);
  p_x_p_2_x_2_p_4_x_4_real =
      operand_a_real * operand_b_real - operand_a_imag * operand_b_imag;
  p_x_p_2_x_2_p_4_x_4_imag =
      operand_a_real * operand_b_imag + operand_a_imag * operand_b_real;

  // step 3-6 Compute x_divided * (p + x) * (p^2 + x^2) *(p^4 + x^4)
  operand_a_real = to_be_divided_real;
  operand_a_imag = to_be_divided_imag;
  operand_b_real = p_x_p_2_x_2_p_4_x_4_real;
  operand_b_imag = p_x_p_2_x_2_p_4_x_4_imag;
  Expr p_x_p_2_x_2_p_4_x_4_x_divided_real = Expr(dag, init_num);
  Expr p_x_p_2_x_2_p_4_x_4_x_divided_imag = Expr(dag, init_num);
  p_x_p_2_x_2_p_4_x_4_x_divided_real =
      operand_a_real * operand_b_real - operand_a_imag * operand_b_imag;
  p_x_p_2_x_2_p_4_x_4_x_divided_imag =
      operand_a_real * operand_b_imag + operand_a_imag * operand_b_real;

  // step 3-7 Set the output, i.e., the real and imaginary parts of the final result
  setOutput(dag, "r", p_x_p_2_x_2_p_4_x_4_x_divided_real);
  setOutput(dag, "i", p_x_p_2_x_2_p_4_x_4_x_divided_imag);

  // debug
  // setOutput(dag, "r", p_2_x_2_real);
  // setOutput(dag, "i", p_2_x_2_imag);

  // =========================================================================================
  // Program Execution Phase: Encrypt Input, Execute Program, Decrypt Output, Compare Results
  // ======================================== Note 2: The correct spelling should be compileDag
  compileDag(dag);  // Compile the program
  genKeys(dag);     // Generate cryptographic key environment

  // step 1: Pad the input to the target polynomial length and encrypt
  vector<double> vec_p_real;
  vector<double> vec_p_imag;
  vector<double> vec_to_be_divided_real;
  vector<double> vec_to_be_divided_imag;
  vector<double> vec_divide_real;
  vector<double> vec_divide_imag;

  for (auto &item : DFT_p) {
    vec_p_real.push_back(item.real());
    vec_p_imag.push_back(item.imag());
  }
  for (auto &item : DFT_to_be_divided) {
    vec_to_be_divided_real.push_back(item.real());
    vec_to_be_divided_imag.push_back(item.imag());
  }
  for (auto &item : DFT_divide) {
    vec_divide_real.push_back(item.real());
    vec_divide_imag.push_back(item.imag());
  }

  // debug: Print
  // cout << "Modulus DFT Encoding Result:" << endl;
  // for (int i=0; i<length_DFT; i++) {
  //     cout << vec_p_real[i] << " ";
  // }
  // cout << endl;
  // for (int i=0; i<length_DFT; i++) {
  //     cout << vec_p_imag[i] << " ";
  // }
  // cout << endl;
  // cout << "Dividend DFT Encoding Result:" << endl;
  // for (int i=0; i<length_DFT; i++) {
  //     cout << vec_to_be_divided_real[i] << " ";
  // }
  // cout << endl;
  // for (int i=0; i<length_DFT; i++) {
  //     cout << vec_to_be_divided_imag[i] << " ";
  // }
  // cout << endl;
  // cout << "p Minus Divisor DFT Encoding Result:" << endl;
  // for (int i=0; i<length_DFT; i++) {
  //     cout << vec_divide_real[i] << " ";
  // }
  // cout << endl;
  // for (int i=0; i<length_DFT; i++) {
  //     cout << vec_divide_imag[i] << " ";
  // }
  // cout << endl;
  // cout << "Encoding Length: " << vec_p_real.size() << " " << vec_p_imag.size() << "
  // " << vec_to_be_divided_real.size() << " " << vec_to_be_divided_imag.size()
  // << " " << vec_divide_real.size() << " " << vec_divide_imag.size() << endl;

  int padding_poly_length = POLY - length_DFT;
  for (int i = 0; i < padding_poly_length; i++) {
    vec_p_real.push_back(0);
    vec_p_imag.push_back(0);
    vec_to_be_divided_real.push_back(0);
    vec_to_be_divided_imag.push_back(0);
    vec_divide_real.push_back(0);
    vec_divide_imag.push_back(0);
  }

  // debug: All-zero input
  // for (int i=0;i<POLY;i++){
  //     vec_p_real[i] = 0;
  //     vec_p_imag[i] = 0;
  //     vec_to_be_divided_real[i] = 0;
  //     vec_to_be_divided_imag[i] = 0;
  //     vec_divide_real[i] = 0;
  //     vec_divide_imag[i] = 0;
  // }

  Valuation inputs1{{"pr", vec_p_real}};                // Modulus real part
  Valuation inputs2{{"pi", vec_p_imag}};                // Modulus imaginary part
  Valuation inputs3{{"dedr", vec_to_be_divided_real}};  // Dividend real part
  Valuation inputs4{{"dedi", vec_to_be_divided_imag}};  // Dividend imaginary part
  Valuation inputs5{{"dr", vec_divide_real}};           // Divisor real part
  Valuation inputs6{{"di", vec_divide_imag}};           // Divisor imaginary part

  encryptInput(dag, inputs1);
  encryptInput(dag, inputs2);
  encryptInput(dag, inputs3);
  encryptInput(dag, inputs4);
  encryptInput(dag, inputs5);
  encryptInput(dag, inputs6);

  // step 2: Execute the program
  exeDag(dag);

  // step 3: Decrypt the output
  Valuation outputs;
  decryptOutput(dag, outputs);

  vector<double> result_vector;
  for (const auto &item : outputs) {
    if (std::holds_alternative<std::vector<double>>(item.second)) {
      // vector
      const auto &v = std::get<std::vector<double>>(item.second);
      for (const auto &value_item : v)
        result_vector.push_back(value_item);  // printf("%f ", value_item);
    } else {
      const auto &v = std::get<uint8_t>(item.second);
      printf("uint result %d \n", v);
    }
  }

  // cout << "size of result_vector: " << result_vector.size() << endl;
  /*Retrieve real and imaginary results, verified: imaginary first, real next,
    opposite to the order in homomorphic program definition (Last In First Out)
  */
  vector<double> result_vector_real;
  vector<double> result_vector_imag;
  for (int i = 0; i < POLY; i++) {
    result_vector_imag.push_back(result_vector[i]);
    result_vector_real.push_back(result_vector[i + POLY]);
  }
  // cout << "Retrieved real part result:" << endl;
  // for (int i=0;i<length_DFT;i++){
  //     cout << result_vector_real[i] << " ";
  // }
  // cout << endl;
  // cout << "Retrieved imaginary part result:" << endl;
  // for (int i=0;i<length_DFT;i++){
  //     cout << result_vector_imag[i] << " ";
  // }
  // cout << endl;

  // Combine into a complex vector
  vector<complex<double>> complex_result;
  complex<double> temp;
  for (int i = 0; i < length_DFT; i++) {
    temp = {result_vector_real[i], result_vector_imag[i]};
    complex_result.emplace_back(temp);
  }
  // cout << "Decrypted complex result:" << endl;
  // for (int i = 0; i < complex_result.size(); i++) {
  //     cout << complex_result[i] << " ";
  // }
  // cout << endl;

  // Perform IDFT to obtain the division result
  vector<complex<double>> division_result;
  DFT::dft(complex_result, division_result, length_DFT,
           true);  // IDFT transformation of the division result
  // cout << "Modulus IDFT result:" << endl;
  // for (auto &item : division_result) cout << round(item.real()) << endl; // IDFT transformation result of the division result

  // Convert point value result to integer result
  double division_double = 0;
  for (int i = 0; i < length_DFT; i++) {
    division_double += round(division_result[i].real()) * pow(10, i);
  }
  division_double = division_double / (pow(modulus_p, 8));
  cout << "Final division result: " << division_double << endl;
  cout << "Ideal division result: " << (double)num_to_be_divided / num_origin << endl;

  releaseDag(dag);

}