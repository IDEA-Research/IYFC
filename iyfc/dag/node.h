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
#include <functional>
#include <memory>
#include <ostream>
#include <unordered_set>
#include <vector>

#include "comm_include.h"
#include "data_type.h"
#include "iyfc_dag.h"
#include "node_attr.h"
#include "op_type.h"

namespace iyfc {
class Dag;
class Node;

std::ostream &operator<<(std::ostream &s, const Node &node);

/**
 * @class Node
 * @brief Class Node
 * @details Publicly inherits from AttrList, as a node can have multiple attributes.
 */
class Node : public AttrList, public std::enable_shared_from_this<Node> {
 public:
  // using NodePtr = std::shared_ptr<Node>;

  /**
   * @brief Node constructor.
   * @param [in] op OpType representing the node type.
   * @param [in] dag The DAG to which the node belongs.
   */
  Node(OpType op, Dag *dag);
  ~Node();

  /**
   * @brief Adds an operand to the node.
   * @param [in] Node
   */
  void addOperand(const NodePtr &Node);
  /**
   * @brief Removes an operand from the node.
   * @param [in] Node
   */
  bool eraseOperand(const NodePtr &Node);
  /**
   * @brief Removes all operands from the node.
   */
  void eraseAllOperand();
  /**
   * @brief Replaces an operand with a new one.
   */
  bool replaceOperand(NodePtr old_node, NodePtr new_node);
  void setOperands(std::vector<NodePtr> o);
  std::size_t numOperands() const;
  NodePtr operandAt(size_t i);
  const std::vector<NodePtr> &getOperands() const;

  /**
   * @brief Replaces uses of the node with a new node based on a predicate function.
   * @param [in] Node The new node to replace the uses with.
   */
  void replaceUsesWithIf(NodePtr Node, std::function<bool(const NodePtr &)>);
  void replaceAllUsesWith(NodePtr Node);
  void replaceOtherUsesWith(NodePtr Node);

  std::size_t numUses();
  std::vector<NodePtr> getUses();

  bool isInternal() const;

  const OpType m_op_type;

  Dag *m_dag;  // dag

  // Unique index of the node in its owning DAG
  std::uint64_t m_index;

  friend std::ostream &operator<<(std::ostream &s, const Node &node);

  bool m_use{true};

 private:
  // Similar to a doubly-linked list structure
  std::vector<NodePtr> m_operands;  // use->def chain (unmanaged pointers)
  std::vector<Node *> m_uses;       // def->use chain (managed pointers)

  void addUse(Node *Node);
  bool eraseUse(Node *Node);
};

}  // namespace iyfc
