
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

#include "openfhe_serialization.h"

#include "ciphertext-ser.h"
#include "cryptocontext-ser.h"
#include "decision/openfhe_adapter.h"
#include "key/key-ser.h"
#include "scheme/bfvrns/bfvrns-ser.h"
#include "scheme/ckksrns/ckksrns-ser.h"

using namespace std;
using namespace lbcrypto;

namespace iyfc {

template <class T>
auto getOpenFheTypeTag();

template <>
auto getOpenFheTypeTag<lbcrypto::Ciphertext<DCRTPoly>>() {
  return FheObject::CIPHERTEXT;
}

template <>
auto getOpenFheTypeTag<lbcrypto::Plaintext>() {
  return FheObject::PLAINTEXT;
}

template <>
auto getOpenFheTypeTag<lbcrypto::PrivateKey<DCRTPoly>>() {
  return FheObject::SECRET_KEY;
}

template <>
auto getOpenFheTypeTag<lbcrypto::PublicKey<DCRTPoly>>() {
  return FheObject::PUBLIC_KEY;
}

template <>
auto getOpenFheTypeTag<lbcrypto::CryptoContext<DCRTPoly>>() {
  return FheObject::CONTEXT;
}

/*Data types already supported by openfhe*/
template <class T>
void serializeOpenFheType(const T &obj, FheObject *msg) {
  std::stringstream s;
  Serial::Serialize(obj, s, SerType::BINARY);
  msg->set_data(s.str());
  msg->set_object_type(getOpenFheTypeTag<T>());
  LOG(LOGLEVEL::Trace, "serialize data type %d , size %u", msg->object_type(),
      msg->data().size());
}

template <class T>
void deserializeOpenFheType(T &obj, const FheObject &msg) {
  // Unknown type;
  if (msg.object_type() == FheObject::UNKNOWN) {
    throw std::logic_error("openfhe message type UNKNOWN");
    return;
  }

  if (msg.object_type() != getOpenFheTypeTag<T>()) {
    warn(" err object type %d", msg.object_type());
    throw std::logic_error("openfhe message type mismatch");
  }
  std::stringstream s;
  s.str(msg.data());
  LOG(LOGLEVEL::Trace, "deserialize data type %d , size %u", msg.object_type(),
      msg.data().size());
  Serial::Deserialize(obj, s, SerType::BINARY);
}

unique_ptr<OpenFheValuation> deserialize(const msg::OpenFheValuation &msg) {
  auto obj = std::make_unique<OpenFheValuation>();
  for (const auto &entry : msg.values()) {
    auto &value = obj->operator[](entry.first);

    switch (entry.second.object_type()) {
      case FheObject::CIPHERTEXT: {
        value = lbcrypto::Ciphertext<DCRTPoly>();
        deserializeOpenFheType(get<lbcrypto::Ciphertext<DCRTPoly>>(value),
                               entry.second);
        break;
      }
      case FheObject::PLAINTEXT: {
        value = lbcrypto::Plaintext();
        deserializeOpenFheType(get<lbcrypto::Plaintext>(value), entry.second);
        break;
      }
      default:
        warn("Not a ciphertext or plaintext");
        return {};
    }
  }

  for (const auto &entry : msg.raw_values()) {
    obj->operator[](entry.first) = deserialize(entry.second);
  }

  return obj;
}

unique_ptr<msg::OpenFheValuation> serialize(const OpenFheValuation &obj) {
  auto msg = std::make_unique<msg::OpenFheValuation>();

  auto &values_msg = *msg->mutable_values();
  auto &raw_values_msg = *msg->mutable_raw_values();
  for (const auto &entry : obj) {
    visit(Overloaded{[&](const lbcrypto::Ciphertext<DCRTPoly> &cipher) {
                       serializeOpenFheType(cipher, &values_msg[entry.first]);
                     },
                     [&](const lbcrypto::Plaintext &plain) {
                       serializeOpenFheType(plain, &values_msg[entry.first]);
                     },
                     [&](const std::shared_ptr<ConstantValue<double>> raw) {
                       raw->serialize(raw_values_msg[entry.first]);
                     },
                     [&](const std::shared_ptr<ConstantValue<int64_t>> raw) {
                       raw->serialize(raw_values_msg[entry.first]);
                     }},
          entry.second);
  }

  return msg;
}

void serializeAutomorphismKey(const OpenFhePublic &obj, std::ostream &stream) {
  if (!obj.m_context->SerializeEvalAutomorphismKey(stream, SerType::BINARY)) {
    throw std::logic_error("Error SerializeEvalAutomorphismKey ");
  }
}

void deserializeAutomorphismKey(const OpenFhePublic &obj,
                                std::istream &stream) {
  if (!obj.m_context->DeserializeEvalAutomorphismKey(stream, SerType::BINARY)) {
    throw std::logic_error("Error DeserializeEvalAutomorphismKey");
  }
}

unique_ptr<msg::OpenFhePublic> serialize(const OpenFhePublic &obj) {
  // Serialize a OpenFhePublic object
  auto msg = std::make_unique<msg::OpenFhePublic>();

  // save context
  serializeOpenFheType(obj.m_context, msg->mutable_context());
  // pk
  serializeOpenFheType(obj.m_public_key, msg->mutable_public_key());
  // mult_key
  std::stringstream ss_mult;
  if (obj.m_context->SerializeEvalMultKey(ss_mult, SerType::BINARY) == false) {
    throw std::logic_error(
        "Error writing serialization of the eval mult keys to "
        "key-eval-mult.txt");
  } else {
    msg->mutable_mult_key()->set_data(ss_mult.str());
    LOG(LOGLEVEL::Debug, "serialize mult_key , size %u",
        msg->mult_key().data().size());
  }

  msg->set_final_depth(obj.m_final_depth);
  msg->set_use_bootstrapping(obj.m_use_bootstrapping);
  // rot_key  ,bootstrapping key Handle separately
  if (!obj.m_use_bootstrapping) {
    std::stringstream ss_rot;
    serializeAutomorphismKey(obj, ss_rot);
    msg->mutable_automorphism_key()->set_data(ss_rot.str());
    LOG(LOGLEVEL::Debug, "serialize automorphism_key , size %u",
        msg->automorphism_key().data().size());
  }

  return msg;
}

unique_ptr<OpenFhePublic> deserialize(const msg::OpenFhePublic &msg) {
  // context
  CryptoContext<DCRTPoly> cc;
  deserializeOpenFheType(cc, msg.context());
  PublicKey<DCRTPoly> pk;
  deserializeOpenFheType(pk, msg.public_key());
  // multkey
  std::stringstream ss_mult;
  ss_mult.str(msg.mult_key().data());
  cc->DeserializeEvalMultKey(ss_mult, SerType::BINARY);

  //  only rot_key, bootstrapping key Handle separately
  if (!msg.use_bootstrapping()) {
    if (msg.automorphism_key().data().size() != 0) {
      std::stringstream ss_rot;
      ss_rot.str(msg.automorphism_key().data());
      cc->DeserializeEvalAutomorphismKey(ss_rot, SerType::BINARY);
    }
  }
  auto ptr = std::make_unique<OpenFhePublic>(cc, pk);
  ptr->m_use_bootstrapping = msg.use_bootstrapping();
  ptr->m_final_depth = msg.final_depth();
  return ptr;
}

unique_ptr<msg::OpenFheSecret> serialize(const OpenFheSecret &obj) {
  // Serialize a OpenFheSecret object
  auto msg = std::make_unique<msg::OpenFheSecret>();

  // context
  serializeOpenFheType(obj.m_context, msg->mutable_context());
  serializeOpenFheType(obj.m_secret_key, msg->mutable_secret_key());
  return msg;
}

unique_ptr<OpenFheSecret> deserialize(const msg::OpenFheSecret &msg) {
  // context
  CryptoContext<DCRTPoly> cc;
  deserializeOpenFheType(cc, msg.context());
  PrivateKey<DCRTPoly> sk;
  deserializeOpenFheType(sk, msg.secret_key());
  return std::make_unique<OpenFheSecret>(cc, sk);
}

unique_ptr<msg::OpenFheCkksPara> serialize(const OpenFheCkksPara &obj) {
  auto msg = std::make_unique<msg::OpenFheCkksPara>();

  auto rotations_msg = msg->mutable_rotations();
  rotations_msg->Reserve(obj.rotations.size());
  for (const auto &rotation : obj.rotations) {
    rotations_msg->Add(rotation);
  }
  msg->set_mult_depth(obj.mult_depth);
  msg->set_batch_size(obj.batch_size);
  msg->set_need_bootstrapping(obj.need_bootstrapping);
  msg->set_scaling_mod_size(obj.scaling_mod_size);
  msg->set_first_mod_size(obj.first_mod_size);

  return msg;
}

unique_ptr<OpenFheCkksPara> deserialize(const msg::OpenFheCkksPara &msg) {
  // Create a new OpenFheCkksPara object
  auto obj = std::make_unique<OpenFheCkksPara>();
  obj->rotations = {msg.rotations().begin(), msg.rotations().end()};
  obj->mult_depth = msg.mult_depth();
  obj->batch_size = msg.batch_size();
  obj->need_bootstrapping = msg.need_bootstrapping();
  obj->scaling_mod_size = msg.scaling_mod_size();
  obj->first_mod_size = msg.first_mod_size();

  return obj;
}

unique_ptr<msg::OpenFheBfvPara> serialize(const OpenFheBfvPara &obj) {
  // Create a new protobuf message
  auto msg = std::make_unique<msg::OpenFheBfvPara>();
  auto rotations_msg = msg->mutable_rotations();
  rotations_msg->Reserve(obj.rotations.size());
  for (const auto &rotation : obj.rotations) {
    rotations_msg->Add(rotation);
  }
  msg->set_mult_depth(obj.mult_depth);
  msg->set_batch_size(obj.batch_size);
  msg->set_scaling_mod_size(obj.scaling_mod_size);
  msg->set_plain_modulus(obj.plain_modulus);
  return msg;
}

unique_ptr<OpenFheBfvPara> deserialize(const msg::OpenFheBfvPara &msg) {
  // Create a new OpenFheBfvPara object
  auto obj = std::make_unique<OpenFheBfvPara>();
  obj->rotations = {msg.rotations().begin(), msg.rotations().end()};
  obj->mult_depth = msg.mult_depth();
  obj->batch_size = msg.batch_size();
  obj->scaling_mod_size = msg.scaling_mod_size();
  obj->plain_modulus = msg.plain_modulus();

  return obj;
}

unique_ptr<msg::OpenFheSignature> serialize(const OpenFheSignature &obj) {
  auto msg = std::make_unique<msg::OpenFheSignature>();
  msg->set_batch_size(obj.batch_size);

  // Save the input map
  auto &inputs_map = *msg->mutable_inputs();
  for (auto &[key, info] : obj.inputs) {
    auto &info_msg = inputs_map[key];
    info_msg.set_input_type(static_cast<int32_t>(info.input_type));
  }

  return msg;
}

unique_ptr<OpenFheSignature> deserialize(const msg::OpenFheSignature &msg) {
  unordered_map<string, OpenFheEncodingInfo> inputs;
  for (auto &[key, info_msg] : msg.inputs()) {
    inputs.emplace(
        key, OpenFheEncodingInfo(static_cast<DataType>(info_msg.input_type())));
  }

  return std::make_unique<OpenFheSignature>(msg.batch_size(), move(inputs));
}

/*base*/
int OpenFheAdapterBase::serializeInputInfo(string &str_info) {
  const OpenFheAdapterBase *curr = this;
  // openfhe_valuation
  auto &openfhe_valuation = curr->m_valution;
  if (openfhe_valuation == nullptr) {
    warn(" inputInfoSerialize openfhe_valuation  null !");
    return SER_OPENFHE_INPUT_VALUEATION_NULL;
  }

  unique_ptr<msg::OpenFheValuation> p_valuation = serialize(*openfhe_valuation);
  if (p_valuation->SerializeToString(&str_info)) {
  } else {
    warn("Could not serialize OPENFHEValuation");
    return SER_OPENFHE_INPUT_SERIALIZETOSTRING_FUC_ERR;
  }
  return 0;
}

int OpenFheAdapterBase::deserializeInputInfo(const string &str_info,
                                             bool replace) {
  OpenFheAdapterBase *curr = this;
  msg::OpenFheValuation tmp_info;
  if (tmp_info.ParseFromString(str_info)) {
    unique_ptr<OpenFheValuation> p_valuation = deserialize(tmp_info);
    if (replace)
      m_valution = std::move(p_valuation);
    else
      curr->mergeInput(p_valuation);
  } else {
    warn("deserializeInputInfo Could not parse message");
    return DESER_PARSEFROMSTRING_FUC_ERR;
  }
  return 0;
}

int OpenFheAdapterBase::serializeOutputInfo(string &str_info) {
  const OpenFheAdapterBase *curr = this;
  // openfhe_valuation
  auto &output_en = curr->m_output_en;
  if (output_en == nullptr) {
    warn(" outputInfoSerialize output_en  null !");
    return SER_OPENFHE_OUTPUT_VALUATION_NULL;
  }
  unique_ptr<msg::OpenFheValuation> p_output = serialize(*output_en);
  if (p_output->SerializeToString(&str_info)) {
  } else {
    warn("Could not serialize output OpenFheValuation");
    return SER_OPENFHE_OUTPUT_SERIALIZETOSTRING_FUC_ERR;
  }
  return 0;
}

int OpenFheAdapterBase::deserializeOutputInfo(const string &str_info) {
  OpenFheAdapterBase *curr = this;
  msg::OpenFheValuation tmp_info;
  if (tmp_info.ParseFromString(str_info)) {
    unique_ptr<OpenFheValuation> p_output = deserialize(tmp_info);
    curr->m_output_en = std::move(p_output);
  } else {
    warn("deserializeoutputInfo Could not parse message");
    return DESER_OUTPUT_PARSEFROMSTRING_FUC_ERR;
  }
  return 0;
}

int OpenFheAdapterBase::serializeCommInfo(
    const DagSerializePara &serialize_para, msg::OpenFheAloInfo &tmp_info) {
  const auto *curr = this;
  // SIG
  if (serialize_para.need_sig_info) {
    if (curr->m_signature == nullptr) {
      warn(" serializeAloInfo m_signature  null !");
      return SER_OPENFHE_NEED_SIG_BUT_NULL;
    }
    unique_ptr<msg::OpenFheSignature> p_sig = serialize(*(curr->m_signature));
    tmp_info.mutable_sig()->Swap(p_sig.get());
    LOG(LOGLEVEL::Trace, "after serialize OpenFheSignature");
  }
  // public_ctx
  if (serialize_para.need_exe_ctx || serialize_para.need_encrpt_ctx) {
    auto &public_ctx = std::get<0>(curr->m_openfhe_ctx);
    if (public_ctx == nullptr) {
      warn(" serializeAloInfo public_ctx  null !");
      return SER_OPENFHE_NEED_PUBLIC_BUT_NULL;
    }
    unique_ptr<msg::OpenFhePublic> p_public = serialize(*public_ctx);
    tmp_info.mutable_openfhe_public()->Swap(p_public.get());
    LOG(LOGLEVEL::Trace, "after serialize public_ctx");
  }

  if (serialize_para.need_decrypt_ctx) {
    auto &secret_ctx = std::get<1>(curr->m_openfhe_ctx);
    if (secret_ctx == nullptr) {
      warn(" serializeAloInfo secret_ctx  null !");
      return SER_OPENFHE_NEED_SECRET_BUT_NULL;
    }
    unique_ptr<msg::OpenFheSecret> p_secret = serialize(*secret_ctx);
    tmp_info.mutable_openfhe_secret()->Swap(p_secret.get());
    LOG(LOGLEVEL::Trace, "after serialize OpenFheSecret");
  }
  return 0;
}

int OpenFheAdapterBase::deserializeCommInfo(
    const msg::OpenFheAloInfo tmp_info) {
  unique_ptr<OpenFhePublic> p_public = nullptr;
  unique_ptr<OpenFheSecret> p_secret = nullptr;

  if (tmp_info.has_openfhe_public())
    p_public = deserialize(tmp_info.openfhe_public());
  if (tmp_info.has_openfhe_secret())
    p_secret = deserialize(tmp_info.openfhe_secret());
  // unique move
  m_openfhe_ctx = make_tuple(std::move(p_public), std::move(p_secret));

  if (tmp_info.has_sig()) {
    unique_ptr<OpenFheSignature> p_sig = deserialize(tmp_info.sig());
    m_signature = std::move(p_sig);
  }

  return 0;
}

// openfhe
int OpenFheCkksAdapter::serializeAloInfo(const DagSerializePara &serialize_para,
                                         string &str_info) {
  const OpenFheCkksAdapter *curr = this;
  msg::OpenFheAloInfo tmp_info;

  if (serialize_para.need_genkey_info) {
    if (curr->m_en_params == nullptr) {
      warn(" serializeAloInfo m_en_params  null !");
      return SER_OPENFHE_NEED_GENKEY_BUT_CKKS_PARA_NULL;
    }
    unique_ptr<msg::OpenFheCkksPara> p_ckks_para =
        serialize(*(curr->m_en_params));
    tmp_info.mutable_ckks_parameters()->Swap(p_ckks_para.get());
    LOG(LOGLEVEL::Trace, "after serialize OpenFheCkksPara");
  }
  int comm_ret = serializeCommInfo(serialize_para, tmp_info);
  THROW_ON_ERROR(comm_ret, "serializeCommInfo");

  LOG(LOGLEVEL::Trace, "serialize OpenFheAloInfo msg size %zu",
      tmp_info.ByteSize());

  if (tmp_info.SerializeToString(&str_info)) {
  } else {
    throw std::logic_error(
        "OpenFheAloInfo  SerializeToString err . Exceeded maximum protobuf "
        "size of 2GB\n");
  }
  return 0;
}

int OpenFheCkksAdapter::deserializeAloInfo(const string &str_info) {
  OpenFheCkksAdapter *curr = this;
  msg::OpenFheAloInfo tmp_info;
  if (tmp_info.ParseFromString(str_info)) {
    int de_ret = deserializeCommInfo(tmp_info);
    THROW_ON_ERROR(de_ret, "deserializeCommInfo");

    if (tmp_info.has_ckks_parameters()) {
      unique_ptr<OpenFheCkksPara> p_ckks_para =
          deserialize(tmp_info.ckks_parameters());
      curr->m_en_params = std::move(p_ckks_para);
    }
  } else {
    warn("deserializeAloInfo Could not parse message");
    return DESER_OPENFHE_INFO_PARSE_ERR;
  }
  return 0;
}

int OpenFheCkksAdapter::serializeBootstrappingKey(std::ostream &stream) {
  auto &public_ctx = std::get<0>(this->m_openfhe_ctx);
  if (public_ctx == nullptr) {
    throw std::logic_error(" serializeBootstrappingKey public_ctx  null!");
  }
  serializeAutomorphismKey(*public_ctx, stream);
  return 0;
}

int OpenFheCkksAdapter::deserializeBootstrappingKey(std::istream &stream) {
  auto &public_ctx = std::get<0>(this->m_openfhe_ctx);
  if (public_ctx == nullptr) {
    throw std::logic_error(" deserializeBootstrappingKey public_ctx  null!");
  }
  deserializeAutomorphismKey(*public_ctx, stream);
  return 0;
}

// bfv
int OpenFheBfvAdapter::serializeAloInfo(const DagSerializePara &serialize_para,
                                        string &str_info) {
  const OpenFheBfvAdapter *curr = this;
  msg::OpenFheAloInfo tmp_info;

  if (serialize_para.need_genkey_info) {
    if (curr->m_en_params == nullptr) {
      warn(" serializeAloInfo m_en_params  null !");
      return SER_OPENFHE_NEED_GENKEY_BUT_CKKS_PARA_NULL;
    }
    unique_ptr<msg::OpenFheBfvPara> p_para = serialize(*(curr->m_en_params));
    tmp_info.mutable_bfv_parameters()->Swap(p_para.get());
    LOG(LOGLEVEL::Trace, "after serialize OpenFheBfvPara");
  }
  int comm_ret = serializeCommInfo(serialize_para, tmp_info);
  THROW_ON_ERROR(comm_ret, "serializeCommInfo");

  LOG(LOGLEVEL::Trace, "serialize OpenFheAloInfo msg size %zu",
      tmp_info.ByteSize());

  if (tmp_info.SerializeToString(&str_info)) {
  } else {
    throw std::logic_error(
        "OpenFheAloInfo  SerializeToString err . Exceeded maximum protobuf "
        "size of 2GB\n");
  }
  return 0;
}

int OpenFheBfvAdapter::deserializeAloInfo(const string &str_info) {
  OpenFheBfvAdapter *curr = this;
  msg::OpenFheAloInfo tmp_info;
  if (tmp_info.ParseFromString(str_info)) {
    int de_ret = deserializeCommInfo(tmp_info);
    THROW_ON_ERROR(de_ret, "deserializeCommInfo");

    if (tmp_info.has_bfv_parameters()) {
      unique_ptr<OpenFheBfvPara> p_para =
          deserialize(tmp_info.bfv_parameters());
      curr->m_en_params = std::move(p_para);
    }
  } else {
    warn("deserializeAloInfo Could not parse message");
    return DESER_OPENFHE_INFO_PARSE_ERR;
  }
  return 0;
}

}  // namespace iyfc
