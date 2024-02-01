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

#include "adapter_interface.h"
#include "comm_include.h"
#include "concrete/concrete.h"
#include "concrete/concrete_value.h"

namespace iyfc {
/**
 * @class ConcreteAdapter
 * @brief Inherited from AdapterInterface
 * @details Includes interfaces for genKeys, encrypt, decrypt, execute, and serialization specific to the Concrete library.
 */
class ConcreteAdapter : public AdapterInterface {
 public:
  ConcreteAdapter() {}
  virtual ~ConcreteAdapter() {}
  virtual int encrypt(const Valuation &inputs, bool replace);
  virtual int decrypt();
  virtual int execute(Dag &dag);
  virtual int setParaAndSig(
      std::shared_ptr<ParametersInterface> ptr_parameters);
  virtual int genKeys(const std::string &alo_name, const Dag &dag);

  /* Serialization related - defined in proto */
  virtual int serializeAloInfo(const DagSerializePara &serialize_para,
                               string &str_info);
  virtual int deserializeAloInfo(const string &str_info);

  virtual int serializeInputInfo(string &str_info);
  virtual int deserializeInputInfo(const string &str_info,
                                   bool replace = false);
  virtual int serializeOutputInfo(string &str_info);
  virtual int deserializeOutputInfo(const string &str_info);

  int mergeInput(std::unique_ptr<ConcreteValuation> &p_valuation);

 private:
  std::unique_ptr<ConcreteValuation> m_concrete_valution = nullptr;
  std::unique_ptr<ConcreteValuation> m_concrete_output_en = nullptr;

  std::tuple<std::unique_ptr<ConcretePublic>, std::unique_ptr<ConcreteSecret>>
      m_concrete_ctx;
};

}  // namespace iyfc