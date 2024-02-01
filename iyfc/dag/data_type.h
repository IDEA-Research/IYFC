/*
 * Main data types: Cipher, Raw, Plain
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
#include "util/logging.h"

namespace iyfc {

#define DATA_TYPES \
  X(Undef, 0)      \
  X(Cipher, 1)     \
  X(Raw, 2)        \
  X(Plain, 3)


/**
 * @enum DataType
 * @brief Lists all main data types, including Undefined, Cipher, Raw, and Plain.
 */
enum class DataType : std::int32_t {
#define X(type, code) type = code,
  DATA_TYPES
#undef X
};

/**
 * @brief      Get the name of the data type.
 * @details    Returns a string representation of the given DataType.
 *
 * @param[in]  type  The data type.
 * @return     A string representing the data type.
 */
inline std::string getTypeName(DataType type) {
  switch (type) {
#define X(type, code)  \
  case DataType::type: \
    return #type;
    DATA_TYPES
#undef X
    default:
      warn("Invalid DataType");
  }
  return "";
}

}  // namespace iyfc
