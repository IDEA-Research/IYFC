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
#include <stdexcept>
#include <vector>

#include "dag/iyfc_dag.h"
#include "dag/node_map.h"

namespace iyfc {

/**
 * @class DagTraversal
 * @brief Iterator pattern. Subclasses inherit from this class and overload the operator() to process specific nodes.
 * @details Traverse the DAG structure in top-down or bottom-up directions.
 */
class DagTraversal {
 private:
  Dag &dag;

  NodeMap<bool> m_ready;      // Record nodes that can be used for rewriting
  NodeMap<bool> m_processed;  // processed

  template <bool is_forward>
  bool arePredecessorsDone(const NodePtr &node) {
    for (auto &operand : is_forward ? node->getOperands() : node->getUses()) {
      if (!m_processed[operand]) return false;
    }
    return true;
  }

  /**
   * @brief Breadth-first traversal of each node in the DAG
   */
  template <typename Rewriter, bool is_forward>
  void traverse(Rewriter &&rewrite) {
    m_processed.clear();
    m_ready.clear();

    std::vector<NodePtr> m_readyNodes =
        is_forward ? dag.getSources() : dag.getSinks();
    for (auto &node : m_readyNodes) {
      m_ready[node] = true;
    }

    std::vector<NodePtr> vec_check_list;

    // Breadth-first search
    while (m_readyNodes.size() != 0) {
      auto node = m_readyNodes.back();
      m_readyNodes.pop_back();

      // Record op or uses
      vec_check_list.clear();
      for (auto &succ : is_forward ? node->getUses() : node->getOperands()) {
        vec_check_list.push_back(succ);
      }

      rewrite(node);
      if (node != nullptr) m_processed[node] = true;

      // Add branch nodes
      for (auto &leaf : is_forward ? dag.getSources() : dag.getSinks()) {
        if (!m_ready[leaf]) {
          m_readyNodes.push_back(leaf);
          m_ready[leaf] = true;
        }
      }

      // Add nodes after rewriting the current node; uses and ops are also added to the checklist during breadth-first search
      if (node)
        for (auto &succ : is_forward ? node->getUses() : node->getOperands()) {
          vec_check_list.push_back(succ);
        }

      // Add nodes
      for (auto &succ : vec_check_list) {
        if (!m_ready[succ] && arePredecessorsDone<is_forward>(succ)) {
          m_readyNodes.push_back(succ);
          m_ready[succ] = true;
        }
      }
    }
  }

 public:
  DagTraversal(Dag &g) : dag(g), m_ready(g), m_processed(g) {}

  ~DagTraversal() {}

  /**
   * @brief Forward pass
   * @param [in] rewrite Functor class
   */
  template <typename Rewriter>
  void forwardPass(Rewriter &&rewrite) {
    traverse<Rewriter, true>(std::forward<Rewriter>(rewrite));
  }

  /**
   * @brief Backward pass
   * @param [in] rewrite Functor class
   */
  template <typename Rewriter>
  void backwardPass(Rewriter &&rewrite) {
    traverse<Rewriter, false>(std::forward<Rewriter>(rewrite));
  }
};

}  // namespace iyfc
