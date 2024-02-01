
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
#include "dag/iyfc_dag.h"
#include "dag/node_map.h"

namespace iyfc {
/**
 * @class CleanNodeHandler
 * @brief Cleans unused nodes, which are nodes without uses.
 */
class CleanNodeHandler {
 private:
   /**
   * @brief Check if a node of the specified operation type can be cleaned.
   */
  bool isCanCleanType(const OpType &op_code);
  Dag &m_dag;

  bool m_clean_all{false}; /**< Whether to clean all nodes, including those with uses. */

 public:
 /**
 * @brief Constructor for CleanNodeHandler.
 * @param [in] g The DAG to clean nodes from.
 * @param [in] all_node Whether to clean all nodes, including those with uses.
*/
  CleanNodeHandler(Dag &g, bool all_node = false);

  void operator()(NodePtr &node);
};

}  // namespace iyfc
