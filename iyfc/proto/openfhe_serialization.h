
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
#include <string>
#include <variant>
#include "openfhe/comm/openfhe_comm.h"
#include "openfhe/comm/openfhe_public.h"
#include "openfhe/comm/openfhe_secret.h"
#include "openfhe/comm/openfhe_valuation.h"
#include "util/overloaded.h"
using namespace std;


/* openfhe serialize*/
namespace iyfc {

using FheObject = msg::FheObject;

/**
 * OpenFheValuation deserialize
*/
unique_ptr<OpenFheValuation> deserialize(const msg::OpenFheValuation &msg);

/**
 * OpenFheValuation serialize
*/
unique_ptr<msg::OpenFheValuation> serialize(const OpenFheValuation &obj);

/**
 * OpenFhePublic serialize
*/
unique_ptr<msg::OpenFhePublic> serialize(const OpenFhePublic &obj);
/**
 * OpenFhePublic deserialize
*/
unique_ptr<OpenFhePublic> deserialize(const msg::OpenFhePublic &msg);


/**
 * OpenFheSecret serialize
*/
unique_ptr<msg::OpenFheSecret> serialize(const OpenFheSecret &obj);

/**
 * OpenFheSecret deserialize
*/
unique_ptr<OpenFheSecret> deserialize(const msg::OpenFheSecret &msg);


/**
 * OpenFheCkksPara serialize
*/
unique_ptr<msg::OpenFheCkksPara> serialize(const OpenFheCkksPara &obj) ;
/**
 * OpenFheCkksPara deserialize
*/
unique_ptr<OpenFheCkksPara> deserialize(const msg::OpenFheCkksPara &msg) ;

/**
 * OpenFheBfvPara serialize
*/
unique_ptr<msg::OpenFheBfvPara> serialize(const OpenFheBfvPara &obj) ;

/**
 * OpenFheBfvPara deserialize
*/
unique_ptr<OpenFheBfvPara> deserialize(const msg::OpenFheBfvPara &msg) ;

/**
 * OpenFheSignature serialize
*/
unique_ptr<msg::OpenFheSignature> serialize(const OpenFheSignature &obj) ;

/**
 * OpenFheSignature deserialize
*/
unique_ptr<OpenFheSignature> deserialize(const msg::OpenFheSignature &msg) ;


}  // namespace iyfc
