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

#include <memory>
#include <string>
#include <unordered_map>
#include "dag/data_type.h"
#include "proto/openfhe.pb.h"

namespace iyfc {
/**
 * @class OpenFheEncodingInfo
 * @brief Parameters related to the encoding operation in OpenFHE.
*/
struct OpenFheEncodingInfo {
  DataType input_type;

  OpenFheEncodingInfo(DataType input_type) : input_type(input_type) {}
};

/**
 * @class OpenFheSignature
 * @brief Information about inputs_encode in OpenFHE, including batch_size.
*/
struct OpenFheSignature {
  int batch_size;
  std::unordered_map<std::string, OpenFheEncodingInfo> inputs;

  OpenFheSignature(int batch_size,
                   std::unordered_map<std::string, OpenFheEncodingInfo> inputs)
      : batch_size(batch_size), inputs(inputs) {}
};

/**
 * @brief Serialize OpenFheSignature.
*/
std::unique_ptr<msg::OpenFheSignature> serialize(const OpenFheSignature &);

/**
 * @brief Deserialize OpenFheSignature.
*/
std::unique_ptr<OpenFheSignature> deserialize(const msg::OpenFheSignature &);

}  // namespace iyfc
