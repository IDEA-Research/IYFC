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

#pragma once
#include <string>
#include <tuple>
#include "comm_include.h"
#include "seal/alo/ckks_parameters.h"
#include "seal/alo/seal_signature.h"
#include "seal/comm/seal_public.h"
#include "seal/comm/seal_secret.h"
#include "seal/comm/seal_valuation.h"
#include "seal/comm/seal_executor.h"
#include "seal/comm/seal_comm.h"
#include "adapter_interface.h"
#include "seal/alo/bfv_parameters.h"
#include "seal/alo/bfv_handler.h"
#include "alo_register.h"
#include "err_code.h"
#include "proto/iyfc.pb.h"
#include "seal/alo/seal_ckks_handler.h"


namespace iyfc {

/**
 * @class SealCkksAdapter
 * @brief Inherits from AdapterInterface
 * @details Contains interfaces for genKeys, encrypt, decrypt, execute, and serialization specific to the CKKS algorithm in the SEAL library.
 * @todo There is a lot of duplicated code that needs optimization.
 */
class SealCkksAdapter : public AdapterInterface {
 public:
  SealCkksAdapter() {}
  virtual ~SealCkksAdapter() {}
  virtual int encrypt(const Valuation &inputs, bool replace);
  virtual int decrypt();
  virtual int execute(Dag &dag);
  virtual int setParaAndSig(
      std::shared_ptr<ParametersInterface> ptr_parameters);
  virtual int genKeys(const std::string &alo_name, const Dag &dag);

  /*Serialization related. Defined in the proto directory*/
  virtual int serializeAloInfo(const DagSerializePara &serialize_para,
                                string &str_info);
  virtual int deserializeAloInfo(const string &str_info);

  virtual int serializeInputInfo(string &str_info);
  virtual int deserializeInputInfo(const string &str_info, bool repalce = false);
  virtual int serializeOutputInfo(string &str_info);
  virtual int deserializeOutputInfo(const string &str_info);

  int mergeInput(std::unique_ptr<SEALValuation>& p_valuation);

 private:
  std::shared_ptr<SealSignature> m_ckks_signature = nullptr;
  std::shared_ptr<CKKSParameters> m_ckks_en_params = nullptr;

  std::shared_ptr<SEALValuation> m_ckks_valution = nullptr;
  std::shared_ptr<SEALValuation> m_ckks_output_en = nullptr;

  std::tuple<std::unique_ptr<SEALPublic>, std::unique_ptr<SEALSecret>>
      m_seal_ctx;
};


/**
 * @class SealBfvAdapter
 * @brief Inherits from AdapterInterface
 * @details Contains interfaces for genKeys, encrypt, decrypt, execute, and serialization specific to the BFV algorithm in the SEAL library.
 * @todo There is a lot of duplicated code that needs optimization.
 */
class SealBfvAdapter : public AdapterInterface {
 public:
  SealBfvAdapter() {}
  virtual ~SealBfvAdapter() {}
  virtual int encrypt(const Valuation &inputs, bool replace);
  virtual int decrypt();
  virtual int execute(Dag &dag);
  virtual int setParaAndSig(
      std::shared_ptr<ParametersInterface> ptr_parameters);
  virtual int genKeys(const std::string &alo_name, const Dag &dag);

  /*Serialization related. Defined in the proto directory*/
  virtual int serializeAloInfo(const DagSerializePara &serialize_para,
                                string &str_info);
  virtual int deserializeAloInfo(const string &str_info);

  virtual int serializeInputInfo(string &str_info);
  virtual int deserializeInputInfo(const string &str_info, bool repalce = false);
  virtual int serializeOutputInfo(string &str_info);
  virtual int deserializeOutputInfo(const string &str_info);

  int mergeInput(std::unique_ptr<SEALValuation>& p_valuation);

 private:
  std::shared_ptr<SealSignature> m_signature = nullptr;
  std::shared_ptr<BfvParameters> m_en_params = nullptr;

  std::shared_ptr<SEALValuation> m_valution = nullptr;
  std::shared_ptr<SEALValuation> m_output_en = nullptr;

  std::tuple<std::unique_ptr<SEALPublic>, std::unique_ptr<SEALSecret>>
      m_seal_ctx;
};



}  // namespace iyfc