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

#include "concrete/concrete.h"
#include "concrete/concrete_value.h"
#include "concrete/libforc/concrete_header.h"
#include "util/overloaded.h"

using namespace std;

namespace iyfc {
using FheObject = msg::FheObject;

/**
 * @brief ConcreteValuation deserialize
*/
unique_ptr<ConcreteValuation> deserialize(const msg::ConcreteValuation &msg);

/**
 * @brief ConcreteValuation serialize
*/
unique_ptr<msg::ConcreteValuation> serialize(const ConcreteValuation &obj);


/**
 * @brief ConcretePublic serialize
*/
unique_ptr<msg::ConcretePublic> serialize(const ConcretePublic &obj);

/**
 * @brief ConcretePublic deserialize
*/
unique_ptr<ConcretePublic> deserialize(const msg::ConcretePublic &msg);

/**
 * @brief ConcreteSecret serialize
*/
unique_ptr<msg::ConcreteSecret> serialize(const ConcreteSecret &obj);

/**
 * @brief ConcreteSecret deserialize
*/
unique_ptr<ConcreteSecret> deserialize(const msg::ConcreteSecret &msg);

}  // namespace iyfc
