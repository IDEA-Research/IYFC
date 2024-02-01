
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
#include "ckks_config.h"

#include <sstream>
#include <unordered_map>

#include "util/logging.h"

namespace iyfc {

CKKSConfig::CKKSConfig(
    const std::unordered_map<std::string, std::string> &map_config) {
  for (const auto &entry : map_config) {
    const auto &option = entry.first;
    const auto &str_value = entry.second;
    if (option == "security_level") {
      std::istringstream is(str_value);
      is >> m_security_level;
      if (is.bad()) {
        warn("Could not parse unsigned int in security_level= %s",
             str_value.c_str());
      }
    } else if (option == "quantum_safe") {
      std::istringstream is(str_value);
      is >> std::boolalpha >> m_quantum_safe;
      if (is.bad()) {
        warn("Could not parse boolean in quantum_safe=", str_value.c_str());
      }
    } else if (option == "warn_vec_size") {
      std::istringstream is(str_value);
      is >> std::boolalpha >> m_warn_vecsize;
      if (is.bad()) {
        warn(
            "Could not parse boolean in warn_vec_size=%s. Falling "
            "back to default.",
            str_value.c_str());
      }
    } else {
      warn("Unknown option %s. Available options are:\n%s", option.c_str(),
           OPTIONS_HELP_MESSAGE);
    }
  }
}

std::string CKKSConfig::toString(int indent) const {
  auto indentStr = std::string(indent, ' ');
  std::stringstream s;
  s << std::boolalpha;
  s << indentStr << "security_level = " << m_security_level;
  s << '\n';
  s << indentStr << "quantum_safe = " << m_quantum_safe;
  s << '\n';
  s << indentStr << "warn_vec_size = " << m_warn_vecsize;
  return s.str();
}

}  // namespace iyfc
