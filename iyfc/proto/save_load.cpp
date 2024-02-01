

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
#include "save_load.h"

using namespace std;

namespace iyfc {

KnownType load(istream &in) {
  msg::KnownType msg;
  if (msg.ParseFromIstream(&in)) {
    return deserialize(msg);
  } else {
    warn("Could not parse message");
    return {};
  }
}

KnownType loadFromFile(const string &path) {
  ifstream in(path);
  if (in.fail()) {
    warn("Could not open file");
    return {};
  }
  return load(in);
}

KnownType loadFromString(const string &str) {
  msg::KnownType msg;
  if (msg.ParseFromString(str)) {
    return deserialize(msg);
  } else {
    warn("Could not parse message");
    return {};
  }
}

}  // namespace iyfc
