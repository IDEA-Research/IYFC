
/*
 * ConstantValue - Definitions related to constants, used for handling different types of constants
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
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "proto/iyfc.pb.h"
#include "util/logging.h"

namespace iyfc {


/**
 * @class ConstantValue
 * @brief ConstantValue class, the base class for constant values, including serialization, slot expansion, etc.
 */
template <typename T>
class ConstantValue {
 public:
  ConstantValue(std::size_t size);
  virtual ~ConstantValue();
  virtual const std::vector<T> &expand(std::vector<T> &scratch,
                                       std::size_t slots) const = 0;
  virtual void expandTo(std::vector<T> &result, std::size_t slots) const = 0;
  virtual bool isZero() const = 0;
  virtual void serialize(msg::ConstantValue &msg) const = 0;
  virtual void serialize(msg::ConstantInt64Value &msg) const = 0;

 protected:
  std::size_t size;

  void validateSlots(std::size_t slots) const;
};

// Dense constant - Contiguously arranged

/**
 * @class DenseConstantValue
 * @brief DenseConstantValue class, inherits from ConstantValue, representing a dense constant value
 */
template <typename T>
class DenseConstantValue : public ConstantValue<T> {
 public:
  DenseConstantValue(std::size_t size, std::vector<T> vec_values);

  virtual ~DenseConstantValue() {}

  const std::vector<T> &expand(std::vector<T> &scratch,
                               std::size_t slots) const override;

  void expandTo(std::vector<T> &result, std::size_t slots) const override;

  bool isZero() const override;

  void serialize(msg::ConstantValue &msg) const override {
    msg.set_size(this->size);
    auto valuesMsg = msg.mutable_values();
    valuesMsg->Reserve(m_values.size());
    for (const auto &value : m_values) {
      valuesMsg->Add(value);
    }
  }

  void serialize(msg::ConstantInt64Value &msg) const override {
    msg.set_size(this->size);
    auto valuesMsg = msg.mutable_values();
    valuesMsg->Reserve(m_values.size());
    for (const auto &value : m_values) {
      valuesMsg->Add(value);
    }
  }

 private:
  std::vector<T> m_values;
};


/**
 * @class SparseConstantValue
 * @brief SparseConstantValue class, inherits from ConstantValue, representing a sparse constant value
 */
template <typename T>
class SparseConstantValue : public ConstantValue<T> {
 public:
  SparseConstantValue(std::size_t size,
                      std::vector<std::pair<std::uint32_t, T>> vec_values);
  virtual ~SparseConstantValue() {}

  const std::vector<T> &expand(std::vector<T> &scratch,
                               std::size_t slots) const override;

  void expandTo(std::vector<T> &result, std::size_t slots) const override;

  bool isZero() const override;

  void serialize(msg::ConstantValue &msg) const override {
    msg.set_size(this->size);
    for (const auto &pair : m_values) {
      msg.add_sparse_indices(pair.first);
      msg.add_values(pair.second);
    }
  }

  void serialize(msg::ConstantInt64Value &msg) const override {
    msg.set_size(this->size);
    for (const auto &pair : m_values) {
      msg.add_sparse_indices(pair.first);
      msg.add_values(pair.second);
    }
  }

 private:
  std::vector<std::pair<std::uint32_t, T>> m_values;
};

template <typename T>
ConstantValue<T>::ConstantValue(std::size_t size) : size(size) {}

template <typename T>
ConstantValue<T>::~ConstantValue() {}

template <typename T>
void ConstantValue<T>::validateSlots(std::size_t slots) const {
  if (slots < size) {
    warn("Slots must be at least size of constant");
    return;
  }
  if (slots % size != 0) {
    warn("Size must exactly divide slots");
    return;
  }
}

template <typename T>
DenseConstantValue<T>::DenseConstantValue(std::size_t size,
                                          std::vector<T> vec_values)
    : ConstantValue<T>(size), m_values(vec_values) {
  if (size % m_values.size() != 0) {
    warn("DenseConstantValue size must exactly divide size %zu, value.size %zu",
         size, m_values.size());
    throw std::logic_error("DenseConstantValue size must exactly divide size");
    return;
  }
}

template <typename T>
const std::vector<T> &DenseConstantValue<T>::expand(std::vector<T> &scratch,
                                                    std::size_t slots) const {
  this->validateSlots(slots);
  if (m_values.size() == slots) {
    return m_values;
  } else {
    scratch.clear();
    for (int r = slots / m_values.size(); r > 0; --r) {
      scratch.insert(scratch.end(), m_values.begin(), m_values.end());
    }
    return scratch;
  }
}

template <typename T>
void DenseConstantValue<T>::expandTo(std::vector<T> &result,
                                     std::size_t slots) const {
  this->validateSlots(slots);
  result.clear();
  result.reserve(slots);
  for (int r = slots / m_values.size(); r > 0; --r) {
    result.insert(result.end(), m_values.begin(), m_values.end());
  }
}

template <typename T>
bool DenseConstantValue<T>::isZero() const {
  for (T value : m_values) {
    if (value != 0) return false;
  }

  return true;
}

template <typename T>
SparseConstantValue<T>::SparseConstantValue(
    std::size_t size, std::vector<std::pair<std::uint32_t, T>> vec_values)
    : ConstantValue<T>(size), m_values(vec_values) {}

template <typename T>
const std::vector<T> &SparseConstantValue<T>::expand(std::vector<T> &scratch,
                                                     std::size_t slots) const {
  this->validateSlots(slots);
  scratch.clear();
  scratch.resize(slots);
  for (auto &entry : m_values) {
    for (int i = 0; i < slots; i += m_values.size()) {
      scratch.at(entry.first + i) = entry.second;
    }
  }
  return scratch;
}

template <typename T>
void SparseConstantValue<T>::expandTo(std::vector<T> &result,
                                      std::size_t slots) const {
  this->validateSlots(slots);
  result.clear();
  result.resize(slots);
  for (auto &entry : m_values) {
    for (int i = 0; i < slots; i += m_values.size()) {
      result.at(entry.first + i) = entry.second;
    }
  }
}

template <typename T>
bool SparseConstantValue<T>::isZero() const {
  // TODO: this assumes no repeated indices
  for (auto entry : m_values) {
    if (entry.second != 0) return false;
  }
  return true;
}

/**
 * @brief      ConstantValue serialize
 * @details    Serializes ConstantValue
 *
 * @param[in]  obj  ConstantValue<double> &
 * @return     std::unique_ptr<msg::ConstantValue>
 */
std::unique_ptr<msg::ConstantValue> serialize(const ConstantValue<double> &obj);

/**
 * @brief      ConstantInt64Value serialize
 * @details    Serializes ConstantInt64Value
 *
 * @param[in]  obj const ConstantValue<int64_t> &
 * @return     std::unique_ptr<msg::ConstantInt64Value>
 */
std::unique_ptr<msg::ConstantInt64Value> serialize(
    const ConstantValue<int64_t> &obj);

/**
 * @brief      ConstantValue deserialization
 * @details    Deserializes ConstantValue
 *
 * @param[in]  msg const msg::ConstantValue &
 * @return     std::shared_ptr<ConstantValue<double>>
 */
std::shared_ptr<ConstantValue<double>> deserialize(
    const msg::ConstantValue &msg);

/**
 * @brief      ConstantInt64Value deserialization
 * @details    Deserializes ConstantInt64Value
 *
 * @param[in]  msg const msg::ConstantInt64Value &
 * @return     std::shared_ptr<ConstantValue<int64_t>>
 */
std::shared_ptr<ConstantValue<int64_t>> deserialize(
    const msg::ConstantInt64Value &msg);

}  // namespace iyfc
