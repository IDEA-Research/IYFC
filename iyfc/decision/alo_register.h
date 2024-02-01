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
#include <functional>
#include <map>
#include <memory>
#include <string>

#include "adapter_interface.h"
#include "concrete/concrete_handler.h"
#include "concrete_adapter.h"
#include "openfhe/alo/openfhe_bfv_handler.h"
#include "openfhe/alo/openfhe_ckks_handler.h"
#include "openfhe_adapter.h"
#include "parameters_interface.h"
#include "seal/alo/bfv_handler.h"
#include "seal/alo/seal_ckks_handler.h"
#include "seal_adapter.h"

namespace iyfc {
class AdapterInterface;
/**
 * @class AloHandler
 * @brief Algorithm handler base class mainly includes ParametersInterface parameters
 * and AdapterInterface underlying library adaptation, two member variables.
 */
class AloHandler {
 public:
  AloHandler(){};
  std::shared_ptr<ParametersInterface>
      m_parameters;  // Parameters processing related, the specific processing flow is implemented in the algorithm library
  std::shared_ptr<AdapterInterface> m_encrypt;  // Encryption-related
  // std::shared_ptr<ExecutorInterface> m_executor; // Execution-related
};

/**
 * @class Factory class
 * @brief Register reflection mode to get the corresponding AloHandler by name
 */
struct AloFactory {
  /**
   * @class AloRegister
   */
  template <typename T>
  struct AloRegister {
    AloRegister(const std::string& key) {
      AloFactory::get().m_alo_map.emplace(key, &AloRegister<T>::create);
    }

    template <typename... Args>
    AloRegister(const std::string& key, Args... args) {
      AloFactory::get().m_alo_map.emplace(key, [=] { return new T(args...); });
    }
    inline static AloHandler* create() { return new T; }
  };

  inline AloHandler* produce(const std::string& key) {
    if (m_alo_map.find(key) == m_alo_map.end())
      throw std::invalid_argument("the alo name is not exist!");

    return m_alo_map[key]();
  }

  /**
   * @brief Generate a unique Alohandler through the library_algorithm name
   * @param [in] key Library_algorithm string
   */
  std::unique_ptr<AloHandler> produce_unique(const std::string& key) {
    return std::unique_ptr<AloHandler>(produce(key));
  }

  /**
   * @brief Generate a shared Alohandler through the library_algorithm name
   * @param [in] key Library_algorithm string
   */
  std::shared_ptr<AloHandler> produce_shared(const std::string& key) {
    return std::shared_ptr<AloHandler>(produce(key));
  }
  typedef AloHandler* (*FunPtr)();

  inline static AloFactory& get() {
    static AloFactory instance;
    return instance;
  }

 private:
  AloFactory(){};
  AloFactory(const AloFactory&) = delete;
  AloFactory(AloFactory&&) = delete;

  std::map<std::string, FunPtr> m_alo_map;
};

// std::map<std::string, factory::FunPtr> factory::m_alo_map;

#define REGISTER_ALO_VNAME(T) reg_msg_##T##_
#define REGISTER_ALO(T, key, ...) \
  static AloFactory::AloRegister<T> REGISTER_ALO_VNAME(T)(key, ##__VA_ARGS__);

/**
 * @class SealCkksAlo
 * @brief Seal library ckks algorithm
 */
class SealCkksAlo : public AloHandler {
 public:
  SealCkksAlo();
};

/**
 * @class SealBfvAlo
 * @brief Seal library bfv algorithm
 */
class SealBfvAlo : public AloHandler {
 public:
  SealBfvAlo();
};

/**
 * @class ConcreteAlo
 * @brief Concrete library
 */
class ConcreteAlo : public AloHandler {
 public:
  ConcreteAlo();
};

/**
 * @class OpenfheCkksAlo
 * @brief Openfhe library ckks algorithm
 */
class OpenfheCkksAlo : public AloHandler {
 public:
  OpenfheCkksAlo();
};

/**
 * @class OpenfheBfvAlo
 * @brief Openfhe library bfv algorithm
 */
class OpenfheBfvAlo : public AloHandler {
 public:
  OpenfheBfvAlo();
};

// Register seal_ckks
REGISTER_ALO(SealCkksAlo, "seal_ckks")
REGISTER_ALO(SealBfvAlo, "seal_bfv")
REGISTER_ALO(ConcreteAlo, "concrete")
REGISTER_ALO(OpenfheCkksAlo, "openfhe_ckks")
REGISTER_ALO(OpenfheBfvAlo, "openfhe_bfv")

}  // namespace iyfc
   /*
   AloHandler* p = factory::get().produce("seal_ckks");
   p->handle();   //Message1
   */