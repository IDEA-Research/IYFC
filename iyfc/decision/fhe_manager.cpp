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

#include "fhe_manager.h"
#include <iostream>

#include "err_code.h"
namespace iyfc {
int FheManager::InitAloPtr(const string &alo_name) {
  if (!m_ptr_init) {
    m_alo_handler = AloFactory::get().produce_shared(alo_name);
    m_ptr_parameters = m_alo_handler->m_parameters;
    m_alo_adapter = m_alo_handler->m_encrypt;
  }
  m_ptr_init = true;
  return 0;
}

void FheManager::checkAdapter() {
  if (m_alo_adapter == nullptr) {
    throw std::logic_error("FheManager::m_alo_adapter null !");
  }
}

int FheManager::compileAlo(const string &alo_name, Dag & dag) {
  // Process specific algorithm parameters
  if (m_ptr_parameters == nullptr) {
    throw std::logic_error("m_ptr_parameters null !");
  }
  // Process parameters + generate parameters + sign
  int do_compile = m_ptr_parameters->dagTranspile(dag);
  THROW_ON_ERROR(do_compile, "dagTranspile ");
  // After compilation, set the parameters and signature needed for genkeys
  m_alo_adapter->setParaAndSig(m_ptr_parameters);
  return 0;
}

int FheManager::generateKeys(const string &alo_name, Dag & dag) {
  // todo
  return m_alo_adapter->genKeys(alo_name, dag);
}

int FheManager::encryptInput(const string &alo_name, const Valuation &inputs,bool replace) {
  // todo
  return m_alo_adapter->encrypt(inputs,replace);
}

int FheManager::executor(const string &alo_name, Dag& dag) {
  // todo
  return m_alo_adapter->execute(dag);
}

int FheManager::getDecryptOutput(const string &alo_name, Valuation &valuation) {
  // todo
  int de_ret = m_alo_adapter->decrypt();
  THROW_ON_ERROR(de_ret, "ptr decrypt ");
  if (m_alo_adapter->m_output_de == nullptr) {
    throw std::logic_error("m_output_de null !");
  }
  valuation = *(m_alo_adapter->m_output_de);
  return 0;
}

int FheManager::aloInfoSerialize(const DagSerializePara &serialize_para,
                                 const string &alo_name, string &alo_info) {
  checkAdapter();
  return m_alo_adapter->serializeAloInfo(serialize_para, alo_info);
}

int FheManager::loadAloFromMsg(const string &alo_info) {
  checkAdapter();
  return m_alo_adapter->deserializeAloInfo(alo_info);
}

int FheManager::inputInfoSerialize(string &input_info) {
  checkAdapter();
  return m_alo_adapter->serializeInputInfo(input_info);
}

int FheManager::loadInputFromMsg(const string &input_info, bool replace) {
  checkAdapter();
  return m_alo_adapter->deserializeInputInfo(input_info, replace);
}

int FheManager::outputInfoSerialize(string &output_info) {
  checkAdapter();
  return m_alo_adapter->serializeOutputInfo(output_info);
}

int FheManager::loadOutputFromMsg(const string &output_info) {
  checkAdapter();
  return m_alo_adapter->deserializeOutputInfo(output_info);
}

int FheManager::loadBootstrappingKey(std::istream &stream) {
  checkAdapter();
  return m_alo_adapter->deserializeBootstrappingKey(stream);
}

int FheManager::saveBootstrappingKey(std::ostream &stream) {
  checkAdapter();
  return m_alo_adapter->serializeBootstrappingKey(stream);
}

}  // namespace iyfc
