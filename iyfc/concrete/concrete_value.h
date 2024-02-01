/*
 * ConcreteCipher 
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
#include "concrete_handler.h"
#include "libforc/concrete_header.h"
namespace iyfc {

/**
 * @class ConcreteCipher
 * @brief Class for managing Concrete cipher data from the Rust language library.
 */

class ConcreteCipher {
 public:
  ConcreteCipher() {}
  ConcreteCipher(FheUint4 *value_ptr) : m_fhe_value(value_ptr) {}
  ~ConcreteCipher() {
    if (m_fhe_value != nullptr) {
      // Hand Over to Rust for Deallocation
      c_free_fheuint2(m_fhe_value);
    }
  }
  void release() {
    if (m_fhe_value != nullptr) {
      c_free_fheuint2(m_fhe_value);
    }
  }
  FheUint4 *m_fhe_value = nullptr;  // Define Allocation in Rust Library
};

}  // namespace iyfc
