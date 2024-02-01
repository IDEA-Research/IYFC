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
#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <variant>

#include "constant_value.h"
#include "data_type.h"
#include "proto/iyfc.pb.h"
#include "util/logging.h"

namespace iyfc {

// std::monostate is the first alternative: This allows the variant to be default-constructed.
using AttrValue = std::variant<std::monostate, std::uint32_t, std::int32_t,
                               DataType, std::shared_ptr<ConstantValue<double>>,
                               std::shared_ptr<ConstantValue<int64_t>>>;

// Meta-template type testing
template <class T, class TypeList>
struct IsInVariant;
template <class T, class... Ts>
struct IsInVariant<T, std::variant<Ts...>>
    : std::bool_constant<(... || std::is_same<T, Ts>{})> {
};  // Determines if T is one of the types in the variant

using AttrKey = std::uint8_t;

template <std::uint64_t Key, class T>
struct Attr {
  static_assert(IsInVariant<T, AttrValue>::value,
                "Attribute type not in AttrValue");
  static_assert(Key > 0, "Keys must be strictly positive");
  static_assert(Key <= std::numeric_limits<AttrKey>::max(),
                "Key larger than current AttrKey type");

  using Value = T;
  static constexpr AttrKey key = Key;

  // Checks if the given key and value match the attribute key and type.
  static bool isValid(AttrKey k, const AttrValue &v) {
    return k == Key && std::holds_alternative<T>(v);
  }
};

#define IYFC_ATTR                                                 \
  X(RescaleDivisorAttr, std::uint32_t)                            \
  X(RotationAttr, std::uint32_t)                                  \
  X(ConstValueAttr, std::shared_ptr<ConstantValue<double>>)       \
  X(ConstValueInt64Attr, std::shared_ptr<ConstantValue<int64_t>>) \
  X(Uint32ConstAttr, std::uint32_t)                               \
  X(TypeAttr, DataType)                                           \
  X(RangeAttr, std::uint32_t)                                     \
  X(BoolAttr, std::uint32_t)                                      \
  X(EncodeAtScaleAttr, std::uint32_t)                             \
  X(EncodeAtLevelAttr, std::uint32_t)

// Enumeration for attribute type indices
namespace detail {
/**
 * @enum Attribute type indices for nodes.
 */
enum AttrIndex {
  RESERVE_EMPTY_ATTR_KEY = 0,
#define X(name, type) name##Index,
  IYFC_ATTR
#undef X
};
}  // namespace detail

/**
 * @brief Macro expansion to create attribute classes.
 */
#define X(name, type) using name = Attr<detail::name##Index, type>;
IYFC_ATTR
#undef X

/**
 * @brief Check if the provided attribute key and value match any defined attribute type.
 */
bool isValidAttr(AttrKey k, const AttrValue &v);
/**
 * @brief Get the name of an attribute based on its key.
 */
std::string getAttrName(AttrKey k);

/**
 * @class AttrList
 * @brief Linked list class containing a list of node attributes.
 *        Key is AttrKey, and value is AttrValue.
 */
class AttrList {
 public:
  AttrList() : key(0), tail(nullptr) {}

  /**
   * @brief Deserialize the attribute list.
   * @details Defined in iyfc/proto/iyfc_serialization.cpp.
   */
  int loadAttr(const msg::Attribute &msg);

  /**
   * @brief Serialize the attribute list.
   * @details Defined in iyfc/proto/iyfc_serialization.cpp.
   */
  int serializeAttr(std::function<msg::Attribute *()> add_msg) const;

  /**
   * @brief Check if the attribute list has a specific attribute type.
   */
  template <class TAttr>
  bool has() const {
    return has(TAttr::key);
  }

  /**
   * @brief Get the value of the attribute in the list based on its key.
   */
  template <class TAttr>
  const typename TAttr::Value &get() const {
    return std::get<typename TAttr::Value>(get(TAttr::key));
  }

  /**
   * @brief Set the attribute key-value pair in the list at the appropriate position.
   */
  template <class TAttr>
  void set(typename TAttr::Value value) {
    set(TAttr::key, std::move(value));
  }

  void assignAttrFrom(const AttrList &other);

 private:
  AttrKey key;
  AttrValue value;
  std::unique_ptr<AttrList> tail;

  AttrList(AttrKey k, AttrValue v) : key(k), value(std::move(v)) {}

  bool has(AttrKey k) const;
  const AttrValue &get(AttrKey k) const;
  void set(AttrKey k, AttrValue v);
};

}  // namespace iyfc
