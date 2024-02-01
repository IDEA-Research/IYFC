

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

#include "dag/iyfc_dag.h"
#include "dag/node_map.h"
#include "dag/op_type.h"

namespace iyfc {
/**
 * @enum ConstantValueType
 * @brief Supported data types for constants: double, int_64
 */
enum ConstantValueType {
  TYPE_DOUBLE = 1,
  TYPE_INT64 = 2,
};

/**
 * @class U32ToConstant
 * @brief Converts nodes of uint32_t type to either constant_double or constant_int64 type.
 */
class U32ToConstant {
 private:
  Dag &m_dag;
  int m_type{0};

 public:
  /**
   * @brief U32ToConstant constructor
   * @param[in] g
   * @param[in] type The type to convert to: For CKKS, it converts to double_constant, and for BFV, it converts to int_constant.
   */
  U32ToConstant(Dag &g, int type);

  void operator()(NodePtr &node);
};

}  // namespace iyfc
