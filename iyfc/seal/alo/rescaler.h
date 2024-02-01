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
#include "dag/data_type.h"
#include "dag/iyfc_dag.h"
#include "dag/node_map.h"

namespace iyfc {

/**
 * @class Rescaler
 * @class Rescaler operation base class
 */

class Rescaler {
 protected:
  Dag &dag;
  NodeMap<DataType> &type;
  NodeMapOptional<std::uint32_t> &scale;

  Rescaler(Dag &g, NodeMap<DataType> &type,
           NodeMapOptional<std::uint32_t> &scale)
      : dag(g), type(type), scale(scale) {}

  virtual ~Rescaler(){};

  bool isRescaleOp(const OpType &op_code) {
    return (op_code == OpType::Rescale);
  }

  bool isMultiplicationOp(const OpType &op_code) {
    return (op_code == OpType::Mul);
  }

  bool isAdditionOp(const OpType &op_code) {
    return ((op_code == OpType::Add) || (op_code == OpType::Sub));
  }

  NodePtr insertRescale(NodePtr node, std::uint32_t rescale_by) {
    // auto scale = node->getScale();
    auto rescale_node = dag.makeRescale(node, rescale_by);
    type[rescale_node] = type[node];
    scale[rescale_node] = scale[node] - rescale_by;
    node->replaceOtherUsesWith(rescale_node);

    return rescale_node;
  }

  void insertRescaleBetween(NodePtr left_node, NodePtr right_node,
                            std::uint32_t rescale_by) {
    auto rescale_node = dag.makeRescale(left_node, rescale_by);
    type[rescale_node] = type[left_node];
    scale[rescale_node] = scale[left_node] - rescale_by;

    right_node->replaceOperand(left_node, rescale_node);
  }

  //raw node takes the maximum scale of the op
  void handleRawScale(NodePtr node) {
    if (node->numOperands() > 0) {
      int max_scale = 0;
      for (auto &operand : node->getOperands()) {
        if (scale.at(operand) > max_scale) max_scale = scale.at(operand);
      }
      scale[node] = max_scale;
    }
  }
};

}  // namespace iyfc
