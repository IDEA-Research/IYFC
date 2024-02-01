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
#include <unordered_map>

#include "alo_register.h"
#include "comm_include.h"
#include "dag/iyfc_dag.h"

namespace iyfc {
using namespace std;

/**
 * @class FheManager
 * @brief Algorithm handler manager
 * @details Facilitates future extensions, as there may be multiple schemes for switching
 */
class FheManager {
 private:
  bool m_ptr_init{false};
  // Can be extended to vector<ptr> in the future
  std::shared_ptr<AloHandler>
      m_alo_handler;  // Parameters processing related, specific processing implemented in the algorithm library
  std::shared_ptr<ParametersInterface> m_ptr_parameters;
  std::shared_ptr<AdapterInterface>
      m_alo_adapter;  // Currently does not support switching underlying libraries,
                      // can be changed to map<aloname, aloenptr> later
  void checkAdapter();

 public:
  FheManager() {}
  ~FheManager() {}
  // Things to be processed in the specific process
  int InitAloPtr(const string& alo_name);
  int executor(const string& alo_name, Dag& dag);
  int compileAlo(const string& alo_name, Dag& dag);
  int generateKeys(const string& alo_name, Dag& dag);
  int encryptInput(const string& alo_name, const Valuation& inputs,
                   bool replace = false);
  int getDecryptOutput(const string& alo_name, Valuation& valuation);

  // Serialization related
  int aloInfoSerialize(const DagSerializePara& serialize_para,
                       const string& alo_name, string& alo_info);
  int loadAloFromMsg(const string& alo_info);

  int inputInfoSerialize(string& alo_info);
  int loadInputFromMsg(const string& alo_info, bool replace = false);
  int outputInfoSerialize(string& alo_info);
  int loadOutputFromMsg(const string& alo_info);
  int loadBootstrappingKey(std::istream& stream);
  int saveBootstrappingKey(std::ostream& stream);
};

}  // namespace iyfc