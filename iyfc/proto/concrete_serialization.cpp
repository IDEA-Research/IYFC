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
#include "concrete_serialization.h"
#include "concrete/concrete.h"
#include "decision/concrete_adapter.h"
#include "err_code.h"

using namespace std;

namespace iyfc {

unique_ptr<ConcreteValuation> deserialize(const msg::ConcreteValuation &msg) {
  auto obj = make_unique<ConcreteValuation>();

  for (const auto &entry : msg.values()) {
    auto &value = obj->operator[](entry.first);

    switch (entry.second.object_type()) {
      case FheObject::CIPHERTEXT: {
        auto &v = std::get<std::shared_ptr<ConcreteCipher>>(value);
        v = std::make_shared<ConcreteCipher>();
        char *p = const_cast<char *>(entry.second.data().c_str());
        v->m_fhe_value = c_deserialize_fheuint2(p);
        break;
      }
      case FheObject::UINT8_TEXT: {
        auto &v = std::get<uint32_t>(value);
        v = 0;
        if (!entry.second.data().empty()) v = atoi(entry.second.data().c_str());
        break;
      }
      default:
        throw std::logic_error("Not a ciphertext or uint8");
    }
  }
  return obj;
}

unique_ptr<msg::ConcreteValuation> serialize(const ConcreteValuation &obj) {
  auto msg = make_unique<msg::ConcreteValuation>();
  auto &values_msg = *msg->mutable_values();
  for (const auto &entry : obj) {
    visit(Overloaded{[&](const std::shared_ptr<ConcreteCipher> &cipher) {
                       char *p = c_serialize_fheuint2(cipher->m_fhe_value);
                       auto &object_msg = values_msg[entry.first];
                       object_msg.set_object_type(FheObject::CIPHERTEXT);
                       auto data = make_unique<string>(p);
                       object_msg.set_allocated_data(data.release());
                     },
                     [&](const uint8_t &plain) {
                       auto &object_msg = values_msg[entry.first];
                       object_msg.set_object_type(FheObject::UINT8_TEXT);
                       auto data = make_unique<string>(std::to_string(plain));
                       object_msg.set_allocated_data(data.release());
                     }},
          entry.second);
  }
  return msg;
}

unique_ptr<msg::ConcretePublic> serialize(const ConcretePublic &obj) {
  auto msg = make_unique<msg::ConcretePublic>();
  char *p = c_serialize_server_key(obj.m_server_key);
  auto data = make_unique<string>(p);
  msg->set_allocated_server_str(data.release());
  return msg;
}

unique_ptr<ConcretePublic> deserialize(const msg::ConcretePublic &msg) {
  char *p = const_cast<char *>(msg.server_str().c_str());
  ServerKey *p_key = c_deserialize_server_key(p);
  return make_unique<ConcretePublic>(p_key);
}

unique_ptr<msg::ConcreteSecret> serialize(const ConcreteSecret &obj) {
  auto msg = make_unique<msg::ConcreteSecret>();
  char *p = c_serialize_client_key(obj.m_client_key);
  auto data = make_unique<string>(p);
  msg->set_allocated_client_str(data.release());
  return msg;
}

unique_ptr<ConcreteSecret> deserialize(const msg::ConcreteSecret &msg) {
  char *p = const_cast<char *>(msg.client_str().c_str());
  ClientKey *p_key = c_deserialize_client_key(p);
  return make_unique<ConcreteSecret>(p_key);
}

/*concrete serialization related*/

int ConcreteAdapter::serializeAloInfo(const DagSerializePara &serialize_para,
                                      string &str_info) {
  const ConcreteAdapter *curr = this;
  msg::ConcreteInfo tmp_info;
  //
  if (serialize_para.need_exe_ctx) {
    auto &public_ctx = std::get<0>(curr->m_concrete_ctx);
    if (public_ctx == nullptr) {
      warn(" ConcreteAdapter serializeAloInfo public_ctx  null !");
      return SER_CONCRETE_PUBLIC_CTX_NULL;
    }
    unique_ptr<msg::ConcretePublic> p_public = serialize(*public_ctx);
    tmp_info.mutable_concrete_server()->Swap(p_public.get());
  }
  if (serialize_para.need_encrpt_ctx || serialize_para.need_decrypt_ctx) {
    auto &secret_ctx = std::get<1>(curr->m_concrete_ctx);
    if (secret_ctx == nullptr) {
      warn(" ConcreteAdapter serializeAloInfo secret_ctx  null !");
      return SER_CONCRETE_SECRET_CTX_NULL;
    }
    unique_ptr<msg::ConcreteSecret> p_secret = serialize(*secret_ctx);
    tmp_info.mutable_concrete_client()->Swap(p_secret.get());
  }

  if (tmp_info.SerializeToString(&str_info)) {
    // printf("ConcreteAdapter info SerializeToString success ! \n");
  } else {
    warn("Could not serialize ConcreteAdapter");
    return SER_CONCRETE_SerializeToString_FUNC_ERR;
  }
  return 0;
}

int ConcreteAdapter::deserializeAloInfo(const string &str_info) {
  ConcreteAdapter *curr = this;
  msg::ConcreteInfo tmp_info;
  if (tmp_info.ParseFromString(str_info)) {
    // printf("de alo pb %s \n", tmp_info.ShortDebugString().c_str());

    unique_ptr<iyfc::ConcretePublic> p_public = nullptr;
    unique_ptr<iyfc::ConcreteSecret> p_secret = nullptr;

    if (tmp_info.has_concrete_server()) {
      p_public = deserialize(tmp_info.concrete_server());
    }
    if (tmp_info.has_concrete_client()) {
      p_secret = deserialize(tmp_info.concrete_client());
    }
    // unique move
    curr->m_concrete_ctx = make_tuple(std::move(p_public), std::move(p_secret));
  } else {
    warn("deserializeAloInfo ConcreteAdapter Could not parse message");
    return DESER_CONCRETE_ParseFromString_FUNC_ERR;
  }
  return 0;
}

int ConcreteAdapter::serializeInputInfo(string &str_info) {
  const ConcreteAdapter *curr = this;
  // concret_valuation
  auto &concret_valuation = curr->m_concrete_valution;
  if (concret_valuation == nullptr) {
    warn(" inputInfoSerialize concret_valuation  null !");
    return SER_CONCRETE_INPUT_VAL_NULL;
  }
  unique_ptr<msg::ConcreteValuation> p_valuation =
      serialize(*concret_valuation);
  if (p_valuation->SerializeToString(&str_info)) {
    // printf("ConcreteValuation SerializeToString success ! \n");
  } else {
    warn("Could not serialize ConcreteValuation");
    return SER_CONCRETE_INPUT_SerializeToString_ERR;
  }
  return 0;
}

int ConcreteAdapter::deserializeInputInfo(const string &str_info,
                                          bool replace) {
  ConcreteAdapter *curr = this;
  msg::ConcreteValuation tmp_info;
  if (tmp_info.ParseFromString(str_info)) {
    unique_ptr<ConcreteValuation> p_valuation = deserialize(tmp_info);
    //Support multiple parties to construct input together
    if (replace)
      m_concrete_valution = std::move(p_valuation);
    else
      curr->mergeInput(p_valuation);
  } else {
    warn("deserializeInputInfo ConcreteValuation Could not parse message");
    return DESER_CONCRETE_ParseFromString_FUNC_ERR;
  }
  return 0;
}

int ConcreteAdapter::serializeOutputInfo(string &str_info) {
  const ConcreteAdapter *curr = this;
  // concrete_valuation
  auto &output_en = curr->m_concrete_output_en;
  if (output_en == nullptr) {
    warn(" outputInfoSerialize ConcreteValuation output_en  null !");
    return SER_CONCRETE_OUPUT_VAL_NULL;
  }
  unique_ptr<msg::ConcreteValuation> p_output = serialize(*output_en);
  if (p_output->SerializeToString(&str_info)) {
    // printf("ConcreteValuation ouput SerializeToString success ! \n");
  } else {
    warn("Could not serialize output ConcreteValuation");
    return SER_CONCRETE_OUPUT_SerializeToString_ERR;
  }
  return 0;
}

int ConcreteAdapter::deserializeOutputInfo(const string &str_info) {
  ConcreteAdapter *curr = this;
  msg::ConcreteValuation tmp_info;
  if (tmp_info.ParseFromString(str_info)) {
    unique_ptr<ConcreteValuation> p_output = deserialize(tmp_info);
    curr->m_concrete_output_en = std::move(p_output);
  } else {
    warn("deserializeoutputInfo ConcreteValuation  Could not parse message");
    return DESER_CONCRETE_OUTPUT_ParseFromString_FUNC_ERR;
  }
  return 0;
}

}  // namespace iyfc
