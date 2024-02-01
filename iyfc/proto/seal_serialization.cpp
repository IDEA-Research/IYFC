

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
#include "seal_serialization.h"

#include "decision/seal_adapter.h"

using namespace std;

namespace iyfc {

using FheObject = msg::FheObject;

template <class T>
auto getSEALTypeTag();

template <>
auto getSEALTypeTag<seal::Ciphertext>() {
  return FheObject::CIPHERTEXT;
}

template <>
auto getSEALTypeTag<seal::Plaintext>() {
  return FheObject::PLAINTEXT;
}

template <>
auto getSEALTypeTag<seal::SecretKey>() {
  return FheObject::SECRET_KEY;
}

template <>
auto getSEALTypeTag<seal::PublicKey>() {
  return FheObject::PUBLIC_KEY;
}

template <>
auto getSEALTypeTag<seal::GaloisKeys>() {
  return FheObject::GALOIS_KEYS;
}

template <>
auto getSEALTypeTag<seal::RelinKeys>() {
  return FheObject::RELIN_KEYS;
}

template <>
auto getSEALTypeTag<seal::EncryptionParameters>() {
  return FheObject::ENCRYPTION_PARAMETERS;
}

template <class T>
void serializeSEALType(const T &obj, FheObject *msg) {
  // compression mode
  auto max_size = obj.save_size(seal::Serialization::compr_mode_default);
  auto data = std::make_unique<string>();
  data->resize(max_size);
  auto actual_size =
      obj.save(reinterpret_cast<seal::seal_byte *>(&data->operator[](0)),
               max_size, seal::Serialization::compr_mode_default);
  data->resize(actual_size);
  msg->set_allocated_data(data.release());

  // Set the type tag to indicate the SEAL object type
  msg->set_object_type(getSEALTypeTag<T>());
}

template <class T>
void deserializeSEALType(T &obj, const FheObject &msg) {
  // Unknown type;
  if (msg.object_type() == FheObject::UNKNOWN) {
    throw std::logic_error("SEAL message type set to UNKNOWN");
    return;
  }

  // Type of obj is incompatible with the type indicated in msg
  if (msg.object_type() != getSEALTypeTag<T>()) {
    throw std::logic_error("SEAL message type mismatch");
    return;
  }

  // Load the SEAL object
  obj.load(reinterpret_cast<const seal::seal_byte *>(msg.data().c_str()),
           msg.data().size());
}

template <class T>
void deserializeSEALTypeWithContext(const seal::SEALContext &context, T &obj,
                                    const FheObject &msg) {
  // Most SEAL objects require the SEALContext for safe loading
  // Unknown type;
  if (msg.object_type() == FheObject::UNKNOWN) {
    warn("SEAL message type set to UNKNOWN");
    return;
  }

  // Type of obj is incompatible with the type indicated in msg
  if (msg.object_type() != getSEALTypeTag<T>()) {
    warn("SEAL message type mismatch");
    return;
  }

  // Load the SEAL object and check its validity against given context
  obj.load(context,
           reinterpret_cast<const seal::seal_byte *>(msg.data().c_str()),
           msg.data().size());
}

unique_ptr<SEALValuation> deserialize(const msg::SEALValuation &msg) {
  seal::EncryptionParameters enc_params;
  deserializeSEALType(enc_params, msg.encryption_parameters());
  auto context = getSEALContext(enc_params);

  // Create the destination valuation and load the correct type
  auto obj = std::make_unique<SEALValuation>(enc_params);
  for (const auto &entry : msg.values()) {
    auto &value = obj->operator[](entry.first);

    switch (entry.second.object_type()) {
      case FheObject::CIPHERTEXT: {
        value = seal::Ciphertext();
        deserializeSEALTypeWithContext(context, get<seal::Ciphertext>(value),
                                       entry.second);

        break;
      }
      case FheObject::PLAINTEXT: {
        value = seal::Plaintext();
        deserializeSEALTypeWithContext(context, get<seal::Plaintext>(value),
                                       entry.second);
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

unique_ptr<msg::SEALValuation> serialize(const SEALValuation &obj) {
  auto msg = std::make_unique<msg::SEALValuation>();
  serializeSEALType(obj.params, msg->mutable_encryption_parameters());
  auto &values_msg = *msg->mutable_values();
  auto &raw_msg = *msg->mutable_raw_values();
  for (const auto &entry : obj) {
    visit(Overloaded{[&](const seal::Ciphertext &cipher) {
                       serializeSEALType(cipher, &values_msg[entry.first]);
                     },
                     [&](const seal::Plaintext &plain) {
                       serializeSEALType(plain, &values_msg[entry.first]);
                     },
                     [&](const std::shared_ptr<ConstantValue<double>> raw) {
                       raw->serialize(raw_msg[entry.first]);
                     },
                     [&](const std::shared_ptr<ConstantValue<int64_t>> raw) {
                       raw->serialize(raw_msg[entry.first]);
                     }},
          entry.second);
  }

  return msg;
}

unique_ptr<msg::SEALPublic> serialize(const SEALPublic &obj) {
  // Serialize a SEALPublic object
  auto msg = std::make_unique<msg::SEALPublic>();

  // Save the encryption parameters
  serializeSEALType(obj.context.key_context_data()->parms(),
                    msg->mutable_encryption_parameters());
  // printf("after serializeSEALType en
  // \n%zu",msg->encryption_parameters().data().size()); Save the different
  // public keys
  serializeSEALType(obj.publicKey, msg->mutable_public_key());

  // printf("after serializeSEALType pk \n%zu",msg->public_key().data().size());

  // to do As the number of rotations increases, gk will exceed the maximum limit of 2g PB.
  serializeSEALType(obj.galoisKeys, msg->mutable_galois_keys());

  // printf("after serializeSEALType gk
  // \n%zu",msg->galois_keys().data().size());

  serializeSEALType(obj.relinKeys, msg->mutable_relin_keys());

  // printf("after serializeSEALType rk \n%zu",msg->relin_keys().data().size());

  return msg;
}

unique_ptr<SEALPublic> deserialize(const msg::SEALPublic &msg) {
  // Deserialize a SEALPublic object
  // Load the encryption parameters and acquire a SEALContext; this is needed
  // for safe loading of the other objects
  seal::EncryptionParameters enc_params;
  deserializeSEALType(enc_params, msg.encryption_parameters());
  auto context = getSEALContext(enc_params);

  // Load the different public keys
  seal::PublicKey pk;
  deserializeSEALTypeWithContext(context, pk, msg.public_key());
  seal::GaloisKeys gk;
  deserializeSEALTypeWithContext(context, gk, msg.galois_keys());
  seal::RelinKeys rk;
  deserializeSEALTypeWithContext(context, rk, msg.relin_keys());

  if (enc_params.scheme() == seal::scheme_type::ckks) {
    std::shared_ptr<SealEncoderBase> p_en =
        std::make_shared<CkksEncoder>(context);
    return std::make_unique<SEALPublic>(context, pk, gk, rk, p_en);
  } else if (enc_params.scheme() == seal::scheme_type::bfv) {
    std::shared_ptr<SealEncoderBase> p_en =
        std::make_shared<BfvEncoder>(context);
    return std::make_unique<SEALPublic>(context, pk, gk, rk, p_en);
  }
  return {};
}

unique_ptr<msg::SEALSecret> serialize(const SEALSecret &obj) {
  // Serialize a SEALSecret object
  auto msg = std::make_unique<msg::SEALSecret>();

  // Save the encryption parameters
  serializeSEALType(obj.m_context.key_context_data()->parms(),
                    msg->mutable_encryption_parameters());

  // Save the secret key
  serializeSEALType(obj.m_secret_key, msg->mutable_secret_key());
  return msg;
}

unique_ptr<SEALSecret> deserialize(const msg::SEALSecret &msg) {
  // Deserialize a SEALSecret object
  // Load the encryption parameters and acquire a SEALContext; this is needed
  // for safe loading of the other objects
  seal::EncryptionParameters enc_params;
  deserializeSEALType(enc_params, msg.encryption_parameters());
  auto context = getSEALContext(enc_params);

  // Load the secret key
  seal::SecretKey sk;
  deserializeSEALTypeWithContext(context, sk, msg.secret_key());

  if (enc_params.scheme() == seal::scheme_type::ckks) {
    std::shared_ptr<SealEncoderBase> p_en =
        std::make_shared<CkksEncoder>(context);
    return std::make_unique<SEALSecret>(context, sk, p_en);
  } else if (enc_params.scheme() == seal::scheme_type::bfv) {
    std::shared_ptr<SealEncoderBase> p_en =
        std::make_shared<BfvEncoder>(context);
    return std::make_unique<SEALSecret>(context, sk, p_en);
  }

  return {};
}

unique_ptr<msg::CKKSParameters> serialize(const CKKSParameters &obj) {
  // Create a new protobuf message
  auto msg = std::make_unique<msg::CKKSParameters>();

  // Save the prime bit counts
  auto prime_bits_msg = msg->mutable_prime_bits();
  prime_bits_msg->Reserve(obj.prime_bits.size());
  for (const auto &bits : obj.prime_bits) {
    prime_bits_msg->Add(bits);
  }

  // Save the rotations that are needed
  auto rotations_msg = msg->mutable_rotations();
  rotations_msg->Reserve(obj.rotations.size());
  for (const auto &rotation : obj.rotations) {
    rotations_msg->Add(rotation);
  }

  // Save the polynomial modulus degree
  msg->set_poly_modulus_degree(obj.poly_modulus_degree);

  return msg;
}

unique_ptr<CKKSParameters> deserialize(const msg::CKKSParameters &msg) {
  // Create a new CKKSParameters object
  auto obj = std::make_unique<CKKSParameters>();

  // Load the values from the protobuf message
  obj->prime_bits = {msg.prime_bits().begin(), msg.prime_bits().end()};
  obj->rotations = {msg.rotations().begin(), msg.rotations().end()};
  obj->poly_modulus_degree = msg.poly_modulus_degree();

  return obj;
}

unique_ptr<msg::BfvParameters> serialize(const BfvParameters &obj) {
  // Create a new protobuf message
  auto msg = std::make_unique<msg::BfvParameters>();
  // Save the rotations that are needed
  auto rotations_msg = msg->mutable_rotations();
  rotations_msg->Reserve(obj.rotations.size());
  for (const auto &rotation : obj.rotations) {
    rotations_msg->Add(rotation);
  }
  // Save the polynomial modulus degree
  msg->set_poly_modulus_degree(obj.poly_modulus_degree);
  msg->set_plain_modulus(obj.plain_modulus);

  return msg;
}

unique_ptr<BfvParameters> deserialize(const msg::BfvParameters &msg) {
  // Create a new BfvParameters object
  auto obj = std::make_unique<BfvParameters>();
  obj->rotations = {msg.rotations().begin(), msg.rotations().end()};
  obj->poly_modulus_degree = msg.poly_modulus_degree();
  obj->plain_modulus = msg.plain_modulus();

  return obj;
}

unique_ptr<msg::SealSignature> serialize(const SealSignature &obj) {
  // Create a new protobuf message
  auto msg = std::make_unique<msg::SealSignature>();

  // Save the vector size
  msg->set_vec_size(obj.vec_size);

  // Save the input map
  auto &inputs_map = *msg->mutable_inputs();
  for (auto &[key, info] : obj.inputs) {
    auto &info_msg = inputs_map[key];
    info_msg.set_input_type(static_cast<int32_t>(info.input_type));
    info_msg.set_scale(info.scale);
    info_msg.set_level(info.level);
  }

  return msg;
}

unique_ptr<SealSignature> deserialize(const msg::SealSignature &msg) {
  // Create a new map of SealEncodingInfo objects and load the data
  unordered_map<string, SealEncodingInfo> inputs;
  for (auto &[key, info_msg] : msg.inputs()) {
    inputs.emplace(
        key, SealEncodingInfo(static_cast<DataType>(info_msg.input_type()),
                              info_msg.scale(), info_msg.level()));
  }

  // Return a new SealSignature object
  return std::make_unique<SealSignature>(msg.vec_size(), move(inputs));
}

int SealCkksAdapter::serializeAloInfo(const DagSerializePara &serialize_para,
                                      string &str_info) {
  const SealCkksAdapter *curr = this;
  msg::SealCkksInfo tmp_info;

  if (serialize_para.need_genkey_info) {
    if (curr->m_ckks_en_params == nullptr) {
      warn(" serializeAloInfo m_ckks_en_params  null !");
      return SER_SEAL_NEED_GENKEY_BUT_CKKS_PARA_NULL;
    }
    unique_ptr<msg::CKKSParameters> p_ckks_para =
        serialize(*(curr->m_ckks_en_params));
    tmp_info.mutable_ckks_parameters()->Swap(p_ckks_para.get());
  }
  if (serialize_para.need_sig_info) {
    if (curr->m_ckks_signature == nullptr) {
      warn(" serializeAloInfo m_ckks_signature  null !");
      return SER_SEAL_NEED_SIG_BUT_NULL;
    }
    unique_ptr<msg::SealSignature> p_ckks_sig =
        serialize(*(curr->m_ckks_signature));
    tmp_info.mutable_ckks_sig()->Swap(p_ckks_sig.get());
  }
  // public_ctx
  if (serialize_para.need_exe_ctx || serialize_para.need_encrpt_ctx) {
    auto &public_ctx = std::get<0>(curr->m_seal_ctx);
    if (public_ctx == nullptr) {
      warn(" serializeAloInfo public_ctx  null !");
      return SER_SEAL_NEED_PUBLIC_BUT_NULL;
    }
    unique_ptr<msg::SEALPublic> p_public = serialize(*public_ctx);
    tmp_info.mutable_seal_public()->Swap(p_public.get());
  }

  if (serialize_para.need_decrypt_ctx) {
    auto &secret_ctx = std::get<1>(curr->m_seal_ctx);
    if (secret_ctx == nullptr) {
      warn(" serializeAloInfo secret_ctx  null !");
      return SER_SEAL_NEED_SECRET_BUT_NULL;
    }
    unique_ptr<msg::SEALSecret> p_secret = serialize(*secret_ctx);
    tmp_info.mutable_seal_secret()->Swap(p_secret.get());
  }

  if (tmp_info.SerializeToString(&str_info)) {
  } else {
    warn("SealCkksInfo  SerializeToString err\n");
    return SER_SEAL_INFO_TO_STR_ERR;
  }
  return 0;
}

int SealCkksAdapter::deserializeAloInfo(const string &str_info) {
  SealCkksAdapter *curr = this;
  msg::SealCkksInfo tmp_info;
  if (tmp_info.ParseFromString(str_info)) {
    // printf("de alo pb %s \n", tmp_info.ShortDebugString().c_str());

    unique_ptr<SEALPublic> p_public = nullptr;
    unique_ptr<SEALSecret> p_secret = nullptr;

    if (tmp_info.has_seal_public())
      p_public = deserialize(tmp_info.seal_public());
    if (tmp_info.has_seal_secret())
      p_secret = deserialize(tmp_info.seal_secret());
    // unique move
    curr->m_seal_ctx = make_tuple(std::move(p_public), std::move(p_secret));

    if (tmp_info.has_ckks_parameters()) {
      unique_ptr<CKKSParameters> p_ckks_para =
          deserialize(tmp_info.ckks_parameters());
      curr->m_ckks_en_params = std::move(p_ckks_para);
    }

    if (tmp_info.has_ckks_sig()) {
      unique_ptr<SealSignature> p_ckks_sig = deserialize(tmp_info.ckks_sig());
      curr->m_ckks_signature = std::move(p_ckks_sig);
    }
  } else {
    warn("deserializeAloInfo Could not parse message");
    return DESER_SEAL_INFO_PARSE_ERR;
  }
  return 0;
}

int SealCkksAdapter::serializeInputInfo(string &str_info) {
  const SealCkksAdapter *curr = this;
  // seal_valuation
  auto &seal_valuation = curr->m_ckks_valution;
  if (seal_valuation == nullptr) {
    warn(" inputInfoSerialize seal_valuation  null !");
    return SER_SEAL_INPUT_VALUEATION_NULL;
  }

  unique_ptr<msg::SEALValuation> p_valuation = serialize(*seal_valuation);
  if (p_valuation->SerializeToString(&str_info)) {
  } else {
    warn("Could not serialize SEALValuation");
    return SER_SEAL_INPUT_SERIALIZETOSTRING_FUC_ERR;
  }
  return 0;
}

int SealCkksAdapter::deserializeInputInfo(const string &str_info,
                                          bool repalce) {
  SealCkksAdapter *curr = this;
  msg::SEALValuation tmp_info;
  if (tmp_info.ParseFromString(str_info)) {
    unique_ptr<SEALValuation> p_valuation = deserialize(tmp_info);

    // for(const auto & item :tmp_info.values() ){
    //   printf("input name %s , size %lu \n", item.first.c_str(),
    // item.second.data().size());
    // }
    if (repalce)
      m_ckks_valution = std::move(p_valuation);
    else
      curr->mergeInput(p_valuation);
  } else {
    warn("deserializeInputInfo Could not parse message");
    return DESER_PARSEFROMSTRING_FUC_ERR;
  }
  return 0;
}

int SealCkksAdapter::serializeOutputInfo(string &str_info) {
  const SealCkksAdapter *curr = this;
  // seal_valuation
  auto &output_en = curr->m_ckks_output_en;
  if (output_en == nullptr) {
    warn(" outputInfoSerialize output_en  null !");
    return SER_SEAL_OUTPUT_VALUATION_NULL;
  }
  unique_ptr<msg::SEALValuation> p_output = serialize(*output_en);
  if (p_output->SerializeToString(&str_info)) {
    // printf("seal ckks ouput SerializeToString success ! \n");
  } else {
    warn("Could not serialize output SEALValuation");
    return SER_SEAL_OUTPUT_SERIALIZETOSTRING_FUC_ERR;
  }
  return 0;
}

int SealCkksAdapter::deserializeOutputInfo(const string &str_info) {
  SealCkksAdapter *curr = this;
  msg::SEALValuation tmp_info;
  if (tmp_info.ParseFromString(str_info)) {
    unique_ptr<SEALValuation> p_output = deserialize(tmp_info);
    curr->m_ckks_output_en = std::move(p_output);
  } else {
    warn("deserializeoutputInfo Could not parse message");
    return DESER_OUTPUT_PARSEFROMSTRING_FUC_ERR;
  }
  return 0;
}

int SealBfvAdapter::serializeAloInfo(const DagSerializePara &serialize_para,
                                     string &str_info) {
  const SealBfvAdapter *curr = this;
  msg::SealBfvInfo tmp_info;

  if (serialize_para.need_genkey_info) {
    if (curr->m_en_params == nullptr) {
      warn(" serializeAloInfo m_bfv_en_params  null !");
      return SER_SEAL_NEED_GENKEY_BUT_BFV_PARA_NULL;
    }
    unique_ptr<msg::BfvParameters> p_para = serialize(*(curr->m_en_params));
    tmp_info.mutable_bfv_parameters()->Swap(p_para.get());
  }

  if (serialize_para.need_sig_info) {
    if (curr->m_signature == nullptr) {
      warn(" serializeAloInfo m_bfv_signature  null !");
      return SER_SEAL_BFV_NEED_SIG_BUT_NULL;
    }
    unique_ptr<msg::SealSignature> p_bfv_sig = serialize(*(curr->m_signature));
    tmp_info.mutable_bfv_sig()->Swap(p_bfv_sig.get());
  }

  // public_ctx
  if (serialize_para.need_exe_ctx || serialize_para.need_encrpt_ctx) {
    auto &public_ctx = std::get<0>(curr->m_seal_ctx);
    if (public_ctx == nullptr) {
      warn(" bfv serializeAloInfo public_ctx  null !");
      return SER_SEAL_BFV_NEED_PUBLIC_BUT_NULL;
    }
    unique_ptr<msg::SEALPublic> p_public = serialize(*public_ctx);
    tmp_info.mutable_seal_public()->Swap(p_public.get());
  }
  if (serialize_para.need_decrypt_ctx) {
    auto &secret_ctx = std::get<1>(curr->m_seal_ctx);
    if (secret_ctx == nullptr) {
      warn(" serializeAloInfo secret_ctx  null !");
      return SER_SEAL_BFV_NEED_SECRET_BUT_NULL;
    }
    unique_ptr<msg::SEALSecret> p_secret = serialize(*secret_ctx);
    tmp_info.mutable_seal_secret()->Swap(p_secret.get());
  }

  // printf(" se alo pb %s \n", tmp_info.ShortDebugString().c_str());

  if (tmp_info.SerializeToString(&str_info)) {
  } else {
    warn("Could not serialize SealBfvInfo");
    return SER_SEAL_BFV_INFO_TO_STR_ERR;
  }
  return 0;
}

int SealBfvAdapter::deserializeAloInfo(const string &str_info) {
  SealBfvAdapter *curr = this;
  msg::SealBfvInfo tmp_info;
  if (tmp_info.ParseFromString(str_info)) {
    // printf("de alo pb %s \n", tmp_info.ShortDebugString().c_str());

    unique_ptr<SEALPublic> p_public = nullptr;
    unique_ptr<SEALSecret> p_secret = nullptr;

    if (tmp_info.has_seal_public())
      p_public = deserialize(tmp_info.seal_public());
    if (tmp_info.has_seal_secret())
      p_secret = deserialize(tmp_info.seal_secret());
    // unique move
    curr->m_seal_ctx = make_tuple(std::move(p_public), std::move(p_secret));

    if (tmp_info.has_bfv_parameters()) {
      unique_ptr<BfvParameters> p_para = deserialize(tmp_info.bfv_parameters());
      curr->m_en_params = std::move(p_para);
    }

    if (tmp_info.has_bfv_sig()) {
      unique_ptr<SealSignature> p_sig = deserialize(tmp_info.bfv_sig());
      curr->m_signature = std::move(p_sig);
    }
  } else {
    warn("deserializeAloInfo Could not parse message");
    return DESER_SEAL_BFV_INFO_PARSE_ERR;
  }
  return 0;
}

int SealBfvAdapter::serializeInputInfo(string &str_info) {
  const SealBfvAdapter *curr = this;
  // seal_valuation
  auto &seal_valuation = curr->m_valution;
  if (seal_valuation == nullptr) {
    warn(" inputInfoSerialize seal_valuation  null !");
    return SER_SEAL_BFV_INPUT_VALUEATION_NULL;
  }

  unique_ptr<msg::SEALValuation> p_valuation = serialize(*seal_valuation);
  if (p_valuation->SerializeToString(&str_info)) {
  } else {
    warn("Could not serialize SEALValuation");
    return SER_SEAL_BFV_INPUT_SERIALIZETOSTRING_FUC_ERR;
  }
  return 0;
}

int SealBfvAdapter::deserializeInputInfo(const string &str_info, bool replace) {
  SealBfvAdapter *curr = this;
  msg::SEALValuation tmp_info;
  if (tmp_info.ParseFromString(str_info)) {
    unique_ptr<SEALValuation> p_valuation = deserialize(tmp_info);
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

int SealBfvAdapter::serializeOutputInfo(string &str_info) {
  const SealBfvAdapter *curr = this;
  // seal_valuation
  auto &output_en = curr->m_output_en;
  if (output_en == nullptr) {
    warn(" outputInfoSerialize output_en  null !");
    return SER_SEAL_BFV_OUTPUT_VALUATION_NULL;
  }
  unique_ptr<msg::SEALValuation> p_output = serialize(*output_en);
  if (p_output->SerializeToString(&str_info)) {
    // printf("seal ckks ouput SerializeToString success ! \n");
  } else {
    warn("Could not serialize output SEALValuation");
    return SER_SEAL_BFV_OUTPUT_SERIALIZETOSTRING_FUC_ERR;
  }
  return 0;
}

int SealBfvAdapter::deserializeOutputInfo(const string &str_info) {
  SealBfvAdapter *curr = this;
  msg::SEALValuation tmp_info;
  if (tmp_info.ParseFromString(str_info)) {
    unique_ptr<SEALValuation> p_output = deserialize(tmp_info);
    curr->m_output_en = std::move(p_output);
  } else {
    warn("deserializeoutputInfo Could not parse message");
    return DESER_OUTPUT_PARSEFROMSTRING_FUC_ERR;
  }
  return 0;
}

}  // namespace iyfc
