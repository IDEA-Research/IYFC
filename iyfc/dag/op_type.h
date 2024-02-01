
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
#include <stdexcept>
#include <string>

#include "util/logging.h"

namespace iyfc {

#define IYFC_OPS          \
  X(Undef, 0)             \
  X(Input, 1)             \
  X(Output, 2)            \
  X(Constant, 3)          \
  X(U32Constant, 4)       \
  X(Negate, 10)           \
  X(Add, 11)              \
  X(Sub, 12)              \
  X(Mul, 13)              \
  X(Div, 14)              \
  X(Equality, 15)         \
  X(Greater, 16)          \
  X(Smaller, 17)          \
  X(RotateLeftConst, 18)  \
  X(RotateRightConst, 19) \
  X(Relinearize, 50)      \
  X(ModSwitch, 51)        \
  X(Rescale, 52)          \
  X(Encode, 53)

/**
 * @enum OpType
 * @brief Enumerates all operation types for nodes.
 */
enum class OpType {
#define X(op, code) op = code,
  IYFC_OPS
#undef X
};

/**
 * @brief Check if the given operation type is valid.
 * @param op The operation type to check.
 * @return True if the operation type is valid, false otherwise.
 */
inline bool isValidOp(OpType op) {
  switch (op) {
#define X(op, code) case OpType::op:
    IYFC_OPS
#undef X
    return true;
    default:
      return false;
  }
}

/**
 * @brief Get the name of the given operation type.
 * @param op The operation type.
 * @return The name of the operation type as a string.
 */
inline std::string getOpName(OpType op) {
  switch (op) {
#define X(op, code) \
  case OpType::op:  \
    return #op;
    IYFC_OPS
#undef X
    default:
      warn("Invalid op");
  }
  return "";
}

}  // namespace iyfc
