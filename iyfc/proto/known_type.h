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
#pragma once
#include <memory>
#include <stdexcept>
#include <variant>
#include "concrete/concrete.h"
#include "dag/iyfc_dag.h"
#include "seal/alo/ckks_parameters.h"
#include "seal/alo/bfv_parameters.h"
#include "seal/alo/seal_signature.h"
#include "seal/comm/seal_comm.h"
#include "seal/comm/seal_public.h"
#include "seal/comm/seal_secret.h"
#include "seal/comm/seal_valuation.h"
#include "openfhe/alo/openfhe_parameters.h"
#include "openfhe/alo/openfhe_signature.h"
#include "openfhe/comm/openfhe_comm.h"
#include "openfhe/comm/openfhe_public.h"
#include "openfhe/comm/openfhe_secret.h"
#include "openfhe/comm/openfhe_valuation.h"

#include "util/overloaded.h"
#include "proto/known_type.pb.h"
#include "util/logging.h"

#define IYFC_KNOWN_TYPE_TRY_DESERIALIZE(MsgType)                    \
  do {                                                              \
    if (msg.contents().Is<MsgType>()) {                             \
      MsgType inner;                                                \
      if (!msg.contents().UnpackTo(&inner)) {                       \
        warn("Unpacking inner message failed"); \
      }                                                             \
      obj = deserialize(inner);                                     \
      return;                                                       \
    }                                                               \
  } while (false)

namespace iyfc {

using KnownType =
    std::variant<std::unique_ptr<Dag>, std::unique_ptr<DagGroup>,std::unique_ptr<CKKSParameters>,std::unique_ptr<BfvParameters>,
                 std::unique_ptr<SealSignature>, std::unique_ptr<SEALValuation>,
                 std::unique_ptr<SEALPublic>, std::unique_ptr<SEALSecret>,
                 std::unique_ptr<ConcretePublic>,
                 std::unique_ptr<ConcreteSecret>,
                 std::unique_ptr<ConcreteValuation>,
                 std::unique_ptr<OpenFheCkksPara>,std::unique_ptr<OpenFheSignature>, std::unique_ptr<OpenFheValuation>,
                 std::unique_ptr<OpenFhePublic>, std::unique_ptr<OpenFheSecret>>;

/**
 * @brief KnownType deserialize
 * @details Deserialization of known types.
 *          For specific included types, see the KnownType declaration.
*/
KnownType deserialize(const msg::KnownType &msg);

}  // namespace iyfc
