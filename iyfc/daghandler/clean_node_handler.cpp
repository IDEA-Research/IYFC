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
#include "clean_node_handler.h"
namespace iyfc {

CleanNodeHandler::CleanNodeHandler(Dag &g, bool all_node) : m_dag(g),m_clean_all(all_node) {}

bool CleanNodeHandler::isCanCleanType(const OpType &op_code) {
  return (op_code != OpType::Output && op_code != OpType::Input);// Allow invalid Input nodes
  //return (op_code != OpType::Output ); // Require Input nodes to be used
}

void CleanNodeHandler::operator()(
    NodePtr &node) {  // Must only be used with backward pass traversal
  if (node->numUses() == 0 || m_clean_all) {
    // Nodes without any uses should be cleaned
    if (isCanCleanType(node->m_op_type) || m_clean_all) {
      node->eraseAllOperand();
      // Erase the node
      //
      m_dag.eraseSinks(node.get());
      m_dag.eraseSource(node.get());
      node.reset();
    }
  }
}

}  // namespace iyfc
