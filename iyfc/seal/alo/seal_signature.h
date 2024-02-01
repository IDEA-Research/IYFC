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
#include <string>
#include <unordered_map>
#include "dag/data_type.h"

namespace iyfc {

/**
 * @struct SealEncodingInfo
 * @brief Information for SEAL encoding, including input type, scale, and level.
*/
struct SealEncodingInfo {
  DataType input_type;
  int scale;
  int level;

  SealEncodingInfo(DataType input_type, int scale, int level)
      : input_type(input_type), scale(scale), level(level) {}
};

/**
 * @struct SealSignature
 * @brief Signature information for SEAL, containing encoding parameters for input data.
*/
struct SealSignature {
  int vec_size;
  std::unordered_map<std::string, SealEncodingInfo> inputs;

  SealSignature(int vec_size,
                std::unordered_map<std::string, SealEncodingInfo> inputs)
      : vec_size(vec_size), inputs(inputs) {}
};

std::unique_ptr<msg::SealSignature> serialize(const SealSignature &);
std::unique_ptr<SealSignature> deserialize(const msg::SealSignature &);

}  // namespace iyfc
