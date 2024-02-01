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
#include "openfhe.h"
#include <tuple>
#include <variant>
#include "comm_include.h"
#include "dag/iyfc_dag.h"
#include "openfhe/alo/openfhe_parameters.h"
#include "openfhe_executor.h"
#include "openfhe_public.h"
#include "openfhe_secret.h"
#include "openfhe_valuation.h"

namespace iyfc {

void print_moduli_chain(const DCRTPoly &poly);

/**
 * @brief Generate keys for the CKKS scheme.
 * @param [in] abstract_params  OpenFheCkksPara parameters.
*/
std::tuple<std::unique_ptr<OpenFhePublic>, std::unique_ptr<OpenFheSecret>>
generateKeys(const OpenFheCkksPara &abstract_params);

/**
 * @brief Generate keys for the BFV scheme.
 * @param [in] abstract_params  OpenFheBfvPara parameters.
*/
std::tuple<unique_ptr<OpenFhePublic>, unique_ptr<OpenFheSecret>> generateKeys(
    const OpenFheBfvPara &abstract_params);

}  // namespace iyfc