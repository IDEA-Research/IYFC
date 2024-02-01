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

#pragma once
#include <tuple>

#include "concrete_handler.h"
#include "concrete_value.h"
#include "libforc/concrete_header.h"
#include "proto/concrete.pb.h"

namespace iyfc {
// now support cipher uint8 concret
using concreteSchemeValue =
    std::variant<std::shared_ptr<ConcreteCipher>, uint32_t>;

/**
 * @class ConcreteValuation
 * @brief Class representing concrete-related data, including ConcreteCipher types and uint32_t.
 */
class ConcreteValuation {
 public:
  ConcreteValuation() {}
  ~ConcreteValuation() {}
  auto &operator[](const std::string &name) { return m_values[name]; }
  auto begin() { return m_values.begin(); }
  auto begin() const { return m_values.begin(); }
  auto end() { return m_values.end(); }
  auto end() const { return m_values.end(); }

  void addValues(std::unique_ptr<ConcreteValuation> &p_valuation) {
    m_values.insert(p_valuation->begin(), p_valuation->end());
  }

  /**
   * @brief      ConcreteValuation serialize
   * @return     u_size   std::unique_ptr<msg::ConcreteValuation>
   */
  friend std::unique_ptr<msg::ConcreteValuation> serialize(
      const ConcreteValuation &val);

 private:
  std::unordered_map<std::string, concreteSchemeValue> m_values;
};

/**
 * @brief      Deserialize a ConcreteValuation.
 * @param[in]  msg Constant reference to the msg::ConcreteValuation.
 * @return     std::unique_ptr<ConcreteValuation> deserialize
 */
std::unique_ptr<ConcreteValuation> deserialize(
    const msg::ConcreteValuation &msg);


/**
 * @class ConcretePublic
 * @brief Class representing ConcretePublic, mainly containing the execute function.
 */
class ConcretePublic {
 public:
  ConcretePublic(ServerKey *tmp_ptr) : m_server_key(tmp_ptr) {}
  ~ConcretePublic() {
    if (m_server_key) {
      c_free_serverkey(m_server_key);
    }
  }

  /**
   * @brief      ConcreteValuation execute
   * @details    Execute the concrete interface.
   * @param[in]  dag     DAG to be executed.
   * @param[in]  inputs  Constant reference to the input ConcreteValuation.
   * @return     std::unique_ptr<ConcreteValuation> Unique pointer to the result ConcreteValuation after execution.
   */
  std::unique_ptr<ConcreteValuation> execute(Dag &dag,
                                             const ConcreteValuation &inputs);

  /**
   * @brief      ConcretePublic serialize
   * @details    Execute the concrete interface.
   * @return     std::unique_ptr<msg::ConcretePublic>  Unique pointer to the serialized msg::ConcretePublic.
   */
  friend std::unique_ptr<msg::ConcretePublic> serialize(
      const ConcretePublic &val);

 private:
  ServerKey *m_server_key = nullptr;
};

/**
 * @brief      ConcretePublic deserialize
 * @details    Deserialize a ConcretePublic.
 * @param[in]  msg Constant reference to the msg::ConcretePublic.
 * @return     std::unique_ptr<ConcretePublic>  Unique pointer to the deserialized ConcretePublic.
 */

std::unique_ptr<ConcretePublic> deserialize(const msg::ConcretePublic &msg);


/**
 * @class ConcreteSecret
 * @brief Class representing ConcreteSecret, mainly containing the encrypt and decrypt functions.
 */
class ConcreteSecret {
 public:
  ConcreteSecret(ClientKey *tmp_key) : m_client_key(tmp_key) {}
  ~ConcreteSecret() {
    if (m_client_key) {
      c_free_clientkey(m_client_key);
    }
  }
  /**
   * @brief      Concrete encryption.
   * @details    Call encrypt function in ConcreteSecret.
   * @param[in]  inputs Constant reference to the input Valuation.
   * @return      std::unique_ptr<ConcreteValuation>  Unique pointer to the encrypted ConcreteValuation.
   */
  std::unique_ptr<ConcreteValuation> encrypt(const Valuation &inputs);
  /**
   * @brief      Concrete decryption.
   * @details    Call decrypt function in ConcreteSecret
   * @param[in]  enc_outputs const Constant reference to the encrypted ConcreteValuation.
   * @return     std::unique_ptr<Valuation>  Unique pointer to the decrypted Valuation.
   */
  std::unique_ptr<Valuation> decrypt(const ConcreteValuation &enc_outputs);

 private:
  ClientKey *m_client_key = nullptr;

  /**
   * @brief      ConcreteSecret serialize
   * @details    Serialize the ConcreteSecret.
   * @return     std::unique_ptr<msg::ConcreteSecret>  Unique pointer to the serialized msg::ConcreteSecret.
   */
  friend std::unique_ptr<msg::ConcreteSecret> serialize(
      const ConcreteSecret &val);
};

/**
 * @brief      ConcreteSecret deserialize
 * @details    Deserialize a ConcreteSecret.
 * @return     std::unique_ptr<ConcreteSecret>  Unique pointer to the deserialized ConcreteSecret.
 */
std::unique_ptr<ConcreteSecret> deserialize(const msg::ConcreteSecret &);

/**
 * @brief      Generate keys for the concrete algorithm.
 * @return     std::tuple<std::unique_ptr<ConcretePublic>,
 * std::unique_ptr<ConcreteSecret>>  Tuple containing unique pointers to ConcretePublic and ConcreteSecret, representing public and private keys, respectively.
 */
std::tuple<std::unique_ptr<ConcretePublic>, std::unique_ptr<ConcreteSecret>>
concreteGenerateKeys();

}  // namespace iyfc