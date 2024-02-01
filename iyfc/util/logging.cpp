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

#include "logging.h"
#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdlib>

namespace iyfc {

int getUserLogLevel() {
  static int userLogLevel = 0;
  static bool parsed = false;
  if (!parsed) {
    if (const char *envP = std::getenv("IYFC_LOG_LEVEL")) {
      auto envStr = std::string(envP);
      try {
        userLogLevel = std::stoi(envStr);
      } catch (std::invalid_argument e) {
        std::transform(envStr.begin(), envStr.end(), envStr.begin(), ::tolower);
        if (envStr == "silent") {
          userLogLevel = 0;
        } else if (envStr == "info") {
          userLogLevel = (int)LOGLEVEL::Info;
        } else if (envStr == "debug") {
          userLogLevel = (int)LOGLEVEL::Debug;
        } else if (envStr == "trace") {
          userLogLevel = (int)LOGLEVEL::Trace;
        } else {
          std::cerr << "Invalid LogLevel IYFC_LOG_LEVEL=" << envStr
                    << " Defaulting to silent.\n";
          userLogLevel = 0;
        }
      }
    }
    parsed = true;
  }
  return userLogLevel;
}

void LOG(LOGLEVEL LogLevel, const char *fmt, ...) {
  if (getUserLogLevel() >= (int)LogLevel) {
    printf("IYFC: ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    fflush(stdout);
  }
}

bool logLevelLeast(LOGLEVEL LogLevel) {
  return getUserLogLevel() >= (int)LogLevel;
}

void IYFC_SO_EXPORT warn(const char *fmt, ...) {
  fprintf(stderr, "WARNING: ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
  fflush(stderr);
}


}  // namespace iyfc
