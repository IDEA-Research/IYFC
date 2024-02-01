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
 * @class ModSwitcher
 * @brief ModSwitcher node insertion strategy based on level values
 */
class ModSwitcher {
  Dag &dag;
  NodeMap<DataType> &type;
  NodeMapOptional<std::uint32_t> &scale;
  NodeMap<std::uint32_t>
      level;  // maintains the reverse level (leaves have 0, roots have max)
  std::vector<NodePtr> m_encode_nodes;

  NodePtr insertModSwitchNode(NodePtr &node, std::uint32_t termLevel);

  virtual bool isLevelAddOp(const OpType &op_code);

  bool isCipherType(const NodePtr &node) const;

 public:
  /**
   * @brief ModSwitcher constructor
   * @param [in] g dag
   * @param [in] type Node data type
   * @param [in] scale Node scales
   */
  ModSwitcher(Dag &g, NodeMap<DataType> &type,
              NodeMapOptional<std::uint32_t> &scale);

  ~ModSwitcher();

  /**
   * @brief Traverse the DAG and add mod_switch operations where needed
   */
  void operator()(NodePtr &node);
};

/**
 * @class BfvModSwitcher
 * @brief ModSwitcher node insertion strategy based on level values with
 * different criteria for Bfv level evaluation. Overrides isLevelAddOp
 */
class BfvModSwitcher : public ModSwitcher {
 public:
  /**
   * @brief BfvModSwitcher constructor
   * @param [in] g dag
   * @param [in] type Node data type
   * @param [in] scale Node scales
   */
  BfvModSwitcher(Dag &g, NodeMap<DataType> &type,
                 NodeMapOptional<std::uint32_t> &scale)
      : ModSwitcher(g, type, scale) {}

  virtual bool isLevelAddOp(const OpType &op_code);
};

}  // namespace iyfc
