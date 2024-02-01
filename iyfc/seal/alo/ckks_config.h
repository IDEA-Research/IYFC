
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

namespace iyfc {

// Relevant parameter settings
const char *const OPTIONS_HELP_MESSAGE =
    "security_level     - How many bits of security parameters should be selected for. int (default=128)\n"
    "quantum_safe       - Select quantum safe parameters. bool (default=false)\n"
    "warn_vec_size      - Warn about possibly inefficient vector size selection. bool (default=true)";
// clang-format on

/**
 * @class CKKSConfig
 * @brief CKKS DAG strategy configuration.
*/
class CKKSConfig {
 public:
  CKKSConfig() {}
  CKKSConfig(const std::unordered_map<std::string, std::string> &configMap);
  std::string toString(int indent = 0) const;

  uint32_t m_security_level = 128;
  bool m_quantum_safe = false;
  // Warnings
  bool m_warn_vecsize = true;
};

}  // namespace iyfc
