
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
#pragma once
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "known_type.h"
#include "util/logging.h"

namespace iyfc {

KnownType load(std::istream &in);
KnownType loadFromFile(const std::string &path);
KnownType loadFromString(const std::string &str);

template <class T>
T load(std::istream &in) {
  return std::get<T>(load(in));
}

template <class T>
T loadFromFile(const std::string &path) {
  return std::get<T>(loadFromFile(path));
}

template <class T>
T loadFromString(const std::string &str) {
  return std::get<T>(loadFromString(str));
}

namespace detail {
template <class T>
void serializeKnownType(const T &obj, msg::KnownType &msg) {
  auto inner = serialize(obj);
  msg.set_creator("IYFC");
  msg.mutable_contents()->PackFrom(*inner);
}
}  // namespace detail

template <class T>
void save(const T &obj, std::ostream &out) {
  msg::KnownType msg;
  detail::serializeKnownType(obj, msg);
  if (!msg.SerializeToOstream(&out)) {
    warn("Could not serialize message");
    return;
  }
}

template <class T>
void saveToFile(const T &obj, const std::string &path) {
  std::ofstream out(path);
  if (out.fail()) {
    warn("saveToFile Could not open file");
    return;
  }
  save(obj, out);
}

template <class T>
std::string saveToString(const T &obj) {
  msg::KnownType msg;
  detail::serializeKnownType(obj, msg);
  std::string str;
  if (msg.SerializeToString(&str)) {
    return str;
  } else {
    warn("Could not serialize message");
    return {};
  }
}

}  // namespace iyfc
