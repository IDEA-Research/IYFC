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
#include "node.h"
#include <algorithm>
#include <iostream>
#include <utility>

using namespace std;

namespace iyfc {

Node::Node(OpType op, Dag *dag)
    : m_op_type(op), m_dag(dag), m_index(dag->allocateIndex()) {
  m_dag->m_sources.insert(this);
  m_dag->m_sinks.insert(this);
}

Node::~Node() {
  // The node's lifetime may outlive the DAG;
  // The memory may not be released immediately after calling delete dag.
  if (m_dag && m_dag->m_init) {
    for (NodePtr &operand : m_operands) {
      operand->eraseUse(this);
    }

    if (m_operands.empty()) {
      m_dag->m_sources.erase(this);
    }
    if (m_uses.empty()) m_dag->m_sinks.erase(this);
  }
}

void Node::addOperand(const NodePtr &node) {
  // If it has operands, remove it from sources
  if (m_operands.empty()) {
    m_dag->m_sources.erase(this);
  }
  m_operands.emplace_back(node);
  node->addUse(this);
}

bool Node::eraseOperand(const NodePtr &node) {
  auto iter = find(m_operands.begin(), m_operands.end(), node);
  if (iter != m_operands.end()) {
    node->eraseUse(this);
    m_operands.erase(iter);
    if (m_operands.empty()) {
      m_dag->m_sources.insert(this);
    }
    return true;
  }
  return false;
}

void Node::eraseAllOperand() {
  for (auto iter = m_operands.begin(); iter != m_operands.end();) {
    (*iter)->eraseUse(this);
    m_operands.erase(iter);
  }
}

bool Node::replaceOperand(NodePtr old_node, NodePtr new_node) {
  bool replaced = false;
  for (NodePtr &operand : m_operands) {
    if (operand == old_node) {
      operand = new_node;
      old_node->eraseUse(this);
      new_node->addUse(this);
      replaced = true;
    }
  }
  return replaced;
}

void Node::replaceUsesWithIf(NodePtr node,
                             function<bool(const NodePtr &)> predicate) {
  auto this_ptr = shared_from_this();  // TODO: avoid this and similar
                                       // unnecessary reference counting
  for (auto &use : getUses()) {
    if (predicate(use)) {
      use->replaceOperand(this_ptr, node);
    }
  }
}

void Node::replaceAllUsesWith(NodePtr node) {
  replaceUsesWithIf(node, [](const NodePtr &) { return true; });
}

void Node::replaceOtherUsesWith(NodePtr node) {
  replaceUsesWithIf(node, [&](const NodePtr &use) { return use != node; });
}

void Node::setOperands(vector<NodePtr> ops) {
  if (m_operands.empty()) {
    m_dag->m_sources.erase(this);
  }

  for (auto &operand : m_operands) {
    operand->eraseUse(this);
  }
  m_operands = move(ops);
  for (auto &operand : m_operands) {
    operand->addUse(this);
  }

  if (m_operands.empty()) {
    m_dag->m_sources.insert(this);
  }
}

size_t Node::numOperands() const { return m_operands.size(); }

NodePtr Node::operandAt(size_t i) { return m_operands.at(i); }

const vector<NodePtr> &Node::getOperands() const { return m_operands; }

size_t Node::numUses() { return m_uses.size(); }

vector<NodePtr> Node::getUses() {
  vector<NodePtr> vec_uses;
  for (Node *use : m_uses) {
    vec_uses.emplace_back(use->shared_from_this());
  }

  return vec_uses;
}

bool Node::isInternal() const {
  return ((m_operands.size() != 0) && (m_uses.size() != 0));
}

void Node::addUse(Node *node) {
  if (m_uses.empty()) {
    m_dag->m_sinks.erase(this);
  }
  m_uses.emplace_back(node);
}

bool Node::eraseUse(Node *node) {
  auto iter = find(m_uses.begin(), m_uses.end(), node);
  if (iter != m_uses.end()) {
    m_uses.erase(iter);
    if (m_uses.empty()) {
      m_dag->m_sinks.insert(this);
      return true;
    }
  }
  return false;
}

ostream &operator<<(ostream &s, const Node &node) {
  s << node.m_index << ':' << getOpName(node.m_op_type) << '(';
  bool first = true;
  for (const auto &operand : node.getOperands()) {
    if (first) {
      first = false;
    } else {
      s << ',';
    }
    s << operand->m_index;
  }
  s << ')';
  return s;
}

}  // namespace iyfc
