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
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <optional>
#include <variant>
#include <vector>

#include "iyfc_dag.h"
namespace iyfc {

/**
 * @class NodeMapBase
 * @brief Base class for index-node maps.
 */
class NodeMapBase {
 public:
  NodeMapBase(Dag &g) : m_dag(&g) { m_dag->registerNodeMap(this); }
  virtual ~NodeMapBase() { m_dag->unregisterNodeMap(this); }

  NodeMapBase(const NodeMapBase &other) : m_dag(other.m_dag) {
    m_dag->registerNodeMap(this);
  }
  NodeMapBase &operator=(const NodeMapBase &other) = default;

  friend class Dag;

 protected:
  /**
  * @brief   Initialize the node map with the given size.
  * @details size = node_index_size
  */
  void init() { m_dag->initNodeMap(*this); }

  std::uint64_t getIndex(const Node &node) const { return node.m_index; }

 private:
  virtual void resize(std::size_t size) = 0;
  Dag *m_dag;
};

/**
 * @class NodeMap
 * @brief Index-node map class.
 * @details Overloads the [] operator to retrieve values with a template type.
 */
template <class TValue>
class NodeMap : NodeMapBase {
 public:
  NodeMap(Dag &g) : NodeMapBase(g) { init(); }
  virtual ~NodeMap() {}

  // Overloaded [ ] operator to get the value corresponding to the node index.
  TValue &operator[](const Node &Node) { return m_values.at(getIndex(Node)); }

  const TValue &operator[](const Node &Node) const {
    return m_values.at(getIndex(Node));
  }

  std::size_t size() const { return m_values.size(); }

  TValue &operator[](const NodePtr &Node) { return this->operator[](*Node); }

  // const TValue &operator[](const NodePtr &Node);

  void clear() { m_values.assign(m_values.size(), {}); }

 private:
  void resize(std::size_t size) override { m_values.resize(size); }
  // deque can be inserted at both ends
  std::deque<TValue> m_values;
};

/**
 * @class NodeMap<bool>
 * @brief Specialization of NodeMap for boolean values.
 * @details Overloads the [] operator for boolean values.
 */
template <>
class NodeMap<bool> : NodeMapBase {
 public:
  NodeMap(Dag &g) : NodeMapBase(g) { init(); }
  virtual ~NodeMap() {}

  std::vector<bool>::reference operator[](const Node &Node) {
    return m_values.at(getIndex(Node));
  }

  bool operator[](const Node &Node) const {
    return m_values.at(getIndex(Node));
  }

  std::size_t size() const { return m_values.size(); }

  std::vector<bool>::reference operator[](const NodePtr &node) {
    return this->operator[](*node);
  }

  bool operator[](const NodePtr &node) const { return this->operator[](*node); }

  void clear() { m_values.assign(m_values.size(), false); }

 private:
  void resize(std::size_t size) override { m_values.resize(size); }

  std::vector<bool> m_values;
};

/**
 * @class NodeMapOptional
 * @brief Specialization of NodeMap for optional values.
 * @details Overloads the [] operator for optional values.
 */
template <class TOptionalValue>
class NodeMapOptional : NodeMapBase {
 public:
  NodeMapOptional(Dag &g) : NodeMapBase(g) { init(); }
  virtual ~NodeMapOptional() {}

  TOptionalValue &operator[](const Node &Node) {
    auto &value = m_values.at(getIndex(Node));
    if (!value.has_value()) {
      value.emplace();
    }
    return *value;
  }

  TOptionalValue &operator[](const NodePtr &node) {
    return this->operator[](*node);
  }

  TOptionalValue &at(const Node &node) {
    return m_values.at(getIndex(node)).value();
  }

  TOptionalValue &at(const NodePtr &node) { return this->at(*node); }

  bool has(const Node &node) const {
    return m_values.at(getIndex(node)).has_value();
  }

  bool has(const NodePtr &node) const { return has(*node); }

  void clear() { m_values.assign(m_values.size(), std::nullopt); }

 private:
  void resize(std::size_t size) override { m_values.resize(size); }

  std::deque<std::optional<TOptionalValue>> m_values;
};

}  // namespace iyfc
