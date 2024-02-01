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
#include "mod_switcher.h"
#include <map>
#include <vector>
namespace iyfc {

NodePtr ModSwitcher::insertModSwitchNode(NodePtr &node,
                                         std::uint32_t node_level) {
  auto new_node = dag.makeNode(OpType::ModSwitch, {node});
  scale[new_node] = scale[node];
  level[new_node] = node_level;
  return new_node;
}

//CKKS rescale nodes level++
bool ModSwitcher::isLevelAddOp(const OpType &op_code) {
  return (op_code == OpType::Rescale);
}

bool ModSwitcher::isCipherType(const NodePtr &node) const {
  return type[node] == DataType::Cipher;
}

ModSwitcher::ModSwitcher(Dag &g, NodeMap<DataType> &type,
                         NodeMapOptional<std::uint32_t> &scale)
    : dag(g), type(type), scale(scale), level(g) {}

ModSwitcher::~ModSwitcher() {
  auto sources = dag.getSources();
  std::uint32_t max_level = 0;
  for (auto &source : sources) {
    if (level[source] > max_level) max_level = level[source];
  }
  for (auto &source : sources) {
    auto curLevel = max_level - level[source];
    source->set<EncodeAtLevelAttr>(curLevel);
  }

  for (auto &encode : m_encode_nodes) {
    encode->set<EncodeAtLevelAttr>(max_level - level[encode]);
  }
}

void ModSwitcher::operator()(
    NodePtr &node) {  // must only be used with backward pass traversal
  if (node->numUses() == 0) return;

  // we do not want to add modswitch for nodes of type raw
  if (type[node] == DataType::Raw) return;

  if (node->m_op_type == OpType::Encode) {
    m_encode_nodes.push_back(node);
  }
  std::map<std::uint32_t, std::vector<NodePtr>> map_use_levels;  // ordered map
  for (auto &use : node->getUses()) {
    map_use_levels[level[use]].push_back(use);
  }

  std::uint32_t node_level = 0;
  if (map_use_levels.size() > 1) {
    auto use_level = map_use_levels.rbegin();  // max to min
    node_level = use_level->first;
    ++use_level;

    auto temp = node;
    auto temp_level = node_level;
    while (use_level != map_use_levels.rend()) {
      auto expected_level = use_level->first;
      while (temp_level > expected_level) {
        temp = insertModSwitchNode(temp, temp_level);
        --temp_level;
      }
      for (auto &use : use_level->second) {
        use->replaceOperand(node, temp);
      }
      ++use_level;
    }
  } else {
    assert(map_use_levels.size() == 1);
    node_level = map_use_levels.begin()->first;
  }
  if (isLevelAddOp(node->m_op_type)) {
    ++node_level;
  }
  level[node] = node_level;
}

// Evaluate Bfv level
bool BfvModSwitcher::isLevelAddOp(const OpType &op_code) {
  return (op_code == OpType::Mul);
}


}  // namespace iyfc
