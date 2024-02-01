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
#include <errno.h>
#include "adapter_interface.h"
#include "alo_register.h"
#include "comm_include.h"
#include "err_code.h"
#include "openfhe/alo/openfhe_bfv_handler.h"
#include "openfhe/alo/openfhe_ckks_handler.h"
#include "openfhe/alo/openfhe_parameters.h"
#include "openfhe/alo/openfhe_signature.h"
#include "openfhe/comm/openfhe_comm.h"
#include "openfhe/comm/openfhe_executor.h"
#include "openfhe/comm/openfhe_public.h"
#include "openfhe/comm/openfhe_secret.h"
#include "openfhe/comm/openfhe_valuation.h"
#include "proto/iyfc.pb.h"

namespace iyfc {

/**
 * @class OpenFheAdapterBase
 * @brief Inherited from AdapterInterface
 * @details Includes interfaces for genKeys, encrypt, decrypt, execute, and serialization for the OpenFhe library
 */
class OpenFheAdapterBase : public AdapterInterface {
 public:
  OpenFheAdapterBase() {}
  virtual ~OpenFheAdapterBase() {}

  virtual int setParaAndSig(
      std::shared_ptr<ParametersInterface> ptr_parameters) = 0;
  virtual int genKeys(const std::string &alo_name, const Dag &dag) = 0;
  virtual int encrypt(const Valuation &inputs, bool replace) = 0;
  virtual int decrypt() = 0;
  virtual int execute(Dag &dag) = 0;
  /*Serialization-related, defined in the proto folder*/
  virtual int serializeAloInfo(const DagSerializePara &serialize_para,
                               string &str_info)=0;
  virtual int deserializeAloInfo(const string &str_info)=0;

  

  virtual int serializeInputInfo(string &str_info);
  virtual int deserializeInputInfo(const string &str_info, bool repalce = false);
  virtual int serializeOutputInfo(string &str_info);
  virtual int deserializeOutputInfo(const string &str_info);

  int mergeInput(std::unique_ptr<OpenFheValuation> &p_valuation);

 protected:
  
  int serializeCommInfo(const DagSerializePara &serialize_para,msg::OpenFheAloInfo& tmp_info);
  int deserializeCommInfo(const msg::OpenFheAloInfo tmp_info);
  std::shared_ptr<OpenFheSignature> m_signature =
      nullptr;  // Parameters related to makeplaintext
  std::shared_ptr<OpenFheValuation> m_valution = nullptr;
  std::shared_ptr<OpenFheValuation> m_output_en = nullptr;
  std::tuple<std::unique_ptr<OpenFhePublic>, std::unique_ptr<OpenFheSecret>>
      m_openfhe_ctx;
};

/**
 * @class OpenFheCkksAdapter
 * @brief Inherits from OpenFheAdapterBase
 * @details Includes interfaces for genKeys, encrypt, decrypt, execute, and serialization for OpenFhe's ckks algorithm
 */
class OpenFheCkksAdapter : public OpenFheAdapterBase {
 public:
  OpenFheCkksAdapter() {}
  virtual ~OpenFheCkksAdapter() {}
  virtual int setParaAndSig(
      std::shared_ptr<ParametersInterface> ptr_parameters);
  virtual int genKeys(const std::string &alo_name, const Dag &dag);

  virtual int encrypt(const Valuation &inputs,bool replace);
  virtual int decrypt();
  virtual int execute(Dag &dag);

  /*Serialization-related, defined in proto*/
  virtual int serializeAloInfo(const DagSerializePara &serialize_para,
                               string &str_info) ;
  virtual int deserializeAloInfo(const string &str_info);

  // OpenFhe's ckks algorithm supports bootstrapping
  virtual int serializeBootstrappingKey(std::ostream &stream) ;
  virtual int deserializeBootstrappingKey(std::istream &stream) ;

 private:
  std::shared_ptr<OpenFheCkksPara> m_en_params = nullptr;  // Context-related parameters
};


/**
 * @class OpenFheBfvAdapter
 * @brief Inherits from OpenFheAdapterBase
 * @details Includes interfaces for genKeys, encrypt, decrypt, execute, and serialization for OpenFhe's bfv algorithm
 */
class OpenFheBfvAdapter : public OpenFheAdapterBase {
 public:
  OpenFheBfvAdapter() {}
  virtual ~OpenFheBfvAdapter() {}
  virtual int setParaAndSig(
      std::shared_ptr<ParametersInterface> ptr_parameters);
  virtual int genKeys(const std::string &alo_name, const Dag &dag);

  virtual int encrypt(const Valuation &inputs, bool replace);
  virtual int decrypt();
  virtual int execute(Dag &dag);

  /*Serialization-related, defined in proto*/
  virtual int serializeAloInfo(const DagSerializePara &serialize_para,
                               string &str_info);
  virtual int deserializeAloInfo(const string &str_info);

 private:
  std::shared_ptr<OpenFheBfvPara> m_en_params = nullptr;  // Context-related parameters
};

}  // namespace iyfc