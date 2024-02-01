/*
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
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

namespace iyfc {
#define IYFC_SO_EXPORT __attribute__((__visibility__("default")))

class Node;
class Dag;
class Expr;

// Parameters related to comparison logic
const int MAX_CMP_NUM = 1024;
const int CMP_P = 7;  // Comparative decomposition basis, preferentially using p=7
const std::unordered_map<int, int> MAP_P2LEN = {{3, 32}, {7, 16}};
const int CMP_BIT_LEN = MAP_P2LEN.at(CMP_P);
const int FFT_N = MAP_P2LEN.at(CMP_P);
const uint32_t CMP_DAG_SIZE = 16384;

// Parameters related to modular chain and algorithm library decision logic
const uint32_t DEFAULT_SCALE = 60;
const uint32_t REDUCE_SCALE = 10;
const uint32_t MAX_SEAL_BITS = 881;
const uint32_t DEFAULT_Q_CNT = 3; // Reserved for the length of input and output modular chains. 3
const uint32_t MAX_MULT_DEPTH_NO_BOOT = 15;
const uint32_t LEVELS_BEFORE_BOOTSTRAP = 6;

// Encryption pre-processing options
enum ENCRPYT_TYPE {
  RAW_DATA = 0,  // Encrypt directly to the original data
  DMP_DATA = 1,  // Comparison == > etc. Operations on data need to be decomposed first
  FFT_DATA = 2,  // FFT first and then encrypt the real and imaginary parts separately for CKKS to control the precision of large number multiplication
};

enum CMP_TYPE {
  LESS = 1,
  EQ = 2,
};

// Serialize stream by type
enum SERIALIZE_DATA_TYPE {
  // ToDo  add other types
  BOOTSTRAPPING_KEY = 10,

};

typedef std::pair<Expr, Expr>
    ComplexExpr;  // Expression of a complex number used in scenarios requiring Fourier transform, querying, division, etc.
typedef std::shared_ptr<Node> NodePtr;
typedef Dag* DagPtr;  // dag.h has many dependencies, so it is not exposed here temporarily
// CKKS, BFV uint8_t reserved for concrete
typedef std::variant<std::vector<double>, double, std::vector<int64_t>, int64_t,
                     uint8_t>
    ValuationType;
typedef std::unordered_map<std::string, ValuationType> Valuation;

// Parameters related to DAG serialization
class IYFC_SO_EXPORT DagSerializePara {
 public:
  DagSerializePara(){}
  DagSerializePara(bool node_info, bool gen_key, bool sig, bool exe_ctx,
                   bool encrpt_ctx, bool decrypt_ctx);
  // Reduce parameter passing through setting function name parameters
  void set_need_node(bool par);
  void set_need_genkey(bool par);
  void set_need_sig(bool par);
  void set_need_exe_ctx(bool par);
  void set_need_encrpt_ctx(bool par);
  void set_need_decrypt_ctx(bool par);

  bool need_node_info{false};  // Original DAG information containing nodes and computation logic
  bool need_genkey_info{false};  // Genkeys need information, e.g., for CKKS it is CKKSParameters
  bool need_sig_info{false};  // Additional information for encryption and decryption, required for encryption and decryption
  bool need_exe_ctx{false};   // Public context required for executing computation logic
  bool need_encrpt_ctx{
      false};  // Encryption environment, SEAL-publicstx, concrete-secretclientctx
  bool need_decrypt_ctx{false};  // Decryption environment, SEAL secret
};

}  // namespace iyfc