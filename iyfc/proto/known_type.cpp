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
#include "known_type.h"
#include "util/logging.h"
using namespace std;

namespace iyfc {

namespace {

inline void dispatchKnownTypeDeserialize(KnownType &obj,
                                         const msg::KnownType &msg) {
  // Try loading msg until the correct type is found
  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::Dag);
  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::DagGroup);
  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::CKKSParameters);
  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::BfvParameters);
  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::SealSignature);
  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::SEALValuation);
  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::SEALPublic);
  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::SEALSecret);
  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::ConcretePublic);
  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::ConcreteSecret);
  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::ConcreteValuation);

  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::OpenFheCkksPara);
  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::OpenFheSignature);
  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::OpenFheValuation);
  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::OpenFhePublic);
  IYFC_KNOWN_TYPE_TRY_DESERIALIZE(msg::OpenFheSecret);

  // This is not a known type
  warn("Unknown inner message type %s", msg.contents().type_url().c_str());
}

}  // namespace

KnownType deserialize(const msg::KnownType &msg) {
  KnownType obj;
  dispatchKnownTypeDeserialize(obj, msg);
  return obj;
}

}  // namespace iyfc
