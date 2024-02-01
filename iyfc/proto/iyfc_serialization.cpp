/*
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
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <stack>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "dag/iyfc_dag.h"
#include "dag/node_attr.h"
#include "dag/node_map.h"
#include "decision/alo_decision.h"
#include "err_code.h"
#include "iyfc_format_version.h"
#include "seal_serialization.h"
#include "util/overloaded.h"

using namespace std;

namespace iyfc {

/**
 * @todo Too much duplicate code needs to be optimized
 */

int AttrList::loadAttr(const msg::Attribute &msg) {
  // Load the attribute key; this encodes the type of the attribute
  AttrKey key = static_cast<AttrKey>(msg.key());

  // A variant that holds the possible value types for attributes
  AttrValue value;

  switch (msg.value_case()) {
    case msg::Attribute::kUint32:
      // The attribute holds a uint32; load it
      value.emplace<uint32_t>(msg.uint32());
      break;
    case msg::Attribute::kInt32:
      // The attribute holds an int32; load it
      value.emplace<int32_t>(msg.int32());
      break;
    case msg::Attribute::kType:
      // The attribute holds a DataType (see iyfc/ir/types.h); load it
      value.emplace<DataType>(static_cast<DataType>(msg.type()));
      break;
    case msg::Attribute::kConstantValue:
      // The attribute holds a constant value; load it
      value.emplace<shared_ptr<ConstantValue<double>>>(
          deserialize(msg.constant_value()));
      break;
    case msg::Attribute::kConstantInt64Value:
      // The attribute holds a constant value; load it
      value.emplace<shared_ptr<ConstantValue<int64_t>>>(
          deserialize(msg.constant_int64_value()));
      break;
    case msg::Attribute::VALUE_NOT_SET:
      // No value is set; set the attribute to std::monostate
      value.emplace<monostate>();
      break;
    default:
      warn("Unknown attribute type");
      return LOAD_INVALID_UNKNOWN_ATTR;
  }

  if (!isValidAttr(key, value)) {
    warn("Invalid attribute encountered");
    return LOAD_INVALID_ATTR;
  }
  set(key, move(value));
  return 0;
}

// Definition for member function AttrList::serializeAttr
int AttrList::serializeAttr(function<msg::Attribute *()> add_msg) const {
  // Nothing to do if key is zero (empty attribute; see node_attr.h)
  if (key == 0) {
    return 0;
  }

  // Go over each attribute in this list
  const AttrList *curr = this;
  do {
    // Get a pointer to a new attribute
    msg::Attribute *msg = add_msg();

    // Set the key and value
    msg->set_key(curr->key);
    visit(Overloaded{[&](const monostate &value) {
                       // This is an empty attribute
                     },
                     [&](const uint32_t &value) { msg->set_uint32(value); },
                     [&](const int32_t &value) { msg->set_int32(value); },
                     [&](const DataType &value) {
                       msg->set_type(static_cast<uint32_t>(value));
                     },
                     [&](const shared_ptr<ConstantValue<double>> &value) {
                       auto valueMsg = serialize(*value);
                       msg->set_allocated_constant_value(valueMsg.release());
                     },
                     [&](const shared_ptr<ConstantValue<int64_t>> &value) {
                       auto valueMsg = serialize(*value);
                       msg->set_allocated_constant_int64_value(
                           valueMsg.release());
                     }},
          curr->value);

    // Move on to the next attribute
    curr = curr->tail.get();
  } while (curr);
  return 0;
}

// Algorithm-related public parameters after serialization decision-making
unique_ptr<msg::Alo> AloDecision::serializeAlo(
    const DagSerializePara &serialize_para) const {
  unique_ptr<msg::Alo> msg = std::make_unique<msg::Alo>();

  const AloDecision *curr = this;

  for (const auto &item : curr->m_libs) {
    auto p_alo = msg->add_alo_items();
    p_alo->set_name(item);
    std::string str_tmp_alo_info;
    if (curr->m_fhe_manager == nullptr) {
      throw std::logic_error(" serializeAlo m_fhe_manager null !");
    }
    int ser_ret = curr->m_fhe_manager->aloInfoSerialize(serialize_para, item,
                                                        str_tmp_alo_info);
    if (ser_ret != 0) {
      throw std::logic_error("aloInfoSerialize err \n");
    }
    p_alo->set_alo_info(str_tmp_alo_info);
  }

  return msg;
}

int AloDecision::loadAloInfoFromMsg(const msg::Alo &msg) {
  // Initialization corresponds to FHE manager
  if (msg.alo_items_size() == 0) {
    warn(" alo_items_size 0 !");
    return LOAD_ALO_ITEM_EMPTY;
  }
  const AloDecision *curr = this;
  // Currently considering only one algorithm situation.
  // For future support of algorithm switching during computation,
  // modifications to the fhe_manager data structure will be required.
  // printf("Alo %s \n", msg.ShortDebugString().c_str());

  for (const auto &msg_item : msg.alo_items()) {
    m_libs.emplace_back(msg_item.name());
    // If the FHE manager does not have a DAG, initialize it and set parameters using the decision algorithm.
    if (curr->m_fhe_manager != nullptr)
      curr->m_fhe_manager->InitAloPtr(msg_item.name());

    curr->m_fhe_manager->loadAloFromMsg(msg_item.alo_info());
  }
  return 0;
}

// Serializes the encrypted input for the algorithm.
unique_ptr<msg::Input> AloDecision::serializeInput() const {
  const AloDecision *curr = this;
  if (curr->m_fhe_manager == nullptr) {
    throw std::logic_error(" serializeInput m_fhe_manager null !");
  }
  unique_ptr<msg::Input> msg = std::make_unique<msg::Input>();

  std::string str_info;
  int ser_ret = curr->m_fhe_manager->inputInfoSerialize(str_info);
  if (ser_ret != 0) {
    throw std::logic_error("serializeInput err");
  }

  msg->set_inputs(str_info);
  return msg;
}

int AloDecision::loadInputFromMsg(const msg::Input &msg, bool replace) {
  if (msg.inputs().empty()) {
    warn(" inputs empty !");
    return LOAD_INPUT_MSG_EMPTY;
  }
  const AloDecision *curr = this;
  curr->m_fhe_manager->loadInputFromMsg(msg.inputs(), replace);
  return 0;
}

unique_ptr<msg::Output> AloDecision::serializeOutput() const {
  const AloDecision *curr = this;
  if (curr->m_fhe_manager == nullptr) {
    throw std::logic_error(" serializeOutput m_fhe_manager null !");
  }
  unique_ptr<msg::Output> msg = std::make_unique<msg::Output>();
  std::string str_info;
  int ser_ret = curr->m_fhe_manager->outputInfoSerialize(str_info);
  if (ser_ret != 0) throw std::logic_error("outputInfoSerialize err!");
  msg->set_outputs(str_info);
  return msg;
}

int AloDecision::loadOutputFromMsg(const msg::Output &msg) {
  if (msg.outputs().empty()) {
    warn(" output empty !");
    return LOAD_OUTPUT_MSG_EMPTY;
  }
  const AloDecision *curr = this;
  curr->m_fhe_manager->loadOutputFromMsg(msg.outputs());
  return 0;
}

// Serializes data of a specified type to an output stream.
int AloDecision::serializeByType(SERIALIZE_DATA_TYPE type,
                                 std::ostream &stream) {
  if (type == BOOTSTRAPPING_KEY) {
    return m_fhe_manager->saveBootstrappingKey(stream);
  } else {
    throw std::logic_error("serializeByType : unsupported type!");
  }
  return 0;
}
int AloDecision::deserializeByType(SERIALIZE_DATA_TYPE type,
                                   std::istream &stream) {
  if (type == BOOTSTRAPPING_KEY) {
    return m_fhe_manager->loadBootstrappingKey(stream);
  } else {
    throw std::logic_error("deserializeByType : unsupported type!");
  }
  return 0;
}

unique_ptr<msg::ConstantValue> serialize(const ConstantValue<double> &obj) {
  // Save a constant value;
  unique_ptr<msg::ConstantValue> msg = std::make_unique<msg::ConstantValue>();
  obj.serialize(*msg);
  return msg;
}

unique_ptr<msg::ConstantInt64Value> serialize(
    const ConstantValue<int64_t> &obj) {
  // Save a constant value;
  unique_ptr<msg::ConstantInt64Value> msg =
      std::make_unique<msg::ConstantInt64Value>();
  obj.serialize(*msg);
  return msg;
}

/*
template <typename T, typename = std::enable_if_t<
                          std::is_same<std::remove_cv_t<T>, double>::value>>
*/
shared_ptr<ConstantValue<double>> deserialize(const msg::ConstantValue &msg) {
  if (msg.size() == 0) {
    warn("Constant must have non-zero size");
    return {};
  }

  size_t size = msg.size();
  if (msg.values_size() == 0) {
    return make_shared<SparseConstantValue<double>>(
        size, vector<pair<uint32_t, double>>{});
  } else if (msg.sparse_indices_size() == 0) {
    // No sparse indices so this is a dense constant
    if (msg.values_size() != 0) {
      vector<double> values(msg.values().begin(), msg.values().end());
      return make_shared<DenseConstantValue<double>>(size, move(values));
    }
  } else {
    if (msg.values_size() != 0) {
      // Must be a sparse constant; check that the data is consistent
      if (msg.sparse_indices_size() != msg.values_size()) {
        warn("Values and sparse indices count mismatch");
        return {};
      }
      // Load the sparse representation
      vector<pair<uint32_t, double>> values;
      auto indexIter = msg.sparse_indices().begin();
      auto valueIter = msg.values().begin();
      while (indexIter != msg.sparse_indices().end()) {
        values.emplace_back(*indexIter, *valueIter);
        ++indexIter;
        ++valueIter;
      }
      return make_shared<SparseConstantValue<double>>(size, move(values));
    }
  }
  return {};
}

/*
template <typename T, typename = std::enable_if_t<
                          std::is_same<std::remove_cv_t<T>, int64_t>::value>>
                          */
shared_ptr<ConstantValue<int64_t>> deserialize(
    const msg::ConstantInt64Value &msg) {
  if (msg.size() == 0) {
    warn("Constant must have non-zero size");
    return {};
  }

  size_t size = msg.size();
  if (msg.values_size() == 0) {
    return make_shared<SparseConstantValue<int64_t>>(
        size, vector<pair<uint32_t, int64_t>>{});
  } else if (msg.sparse_indices_size() == 0) {
    if (msg.values_size() != 0) {
      vector<int64_t> values(msg.values().begin(), msg.values().end());
      return make_shared<DenseConstantValue<int64_t>>(size, move(values));
    }
  } else {
    if (msg.values_size() != 0) {
      if (msg.sparse_indices_size() != msg.values_size()) {
        warn("Values and sparse indices count mismatch");
        return {};
      }
      vector<pair<uint32_t, int64_t>> values;
      auto indexIter = msg.sparse_indices().begin();
      auto valueIter = msg.values().begin();
      while (indexIter != msg.sparse_indices().end()) {
        values.emplace_back(*indexIter, *valueIter);
        ++indexIter;
        ++valueIter;
      }
      return make_shared<SparseConstantValue<int64_t>>(size, move(values));
    }
  }
  return {};
}

void nodesSerialize(const Dag &obj, msg::DagNodes *msg, uint64_t &next_index) {
  msg->set_name(obj.getName());
  unordered_map<Node *, uint64_t> indices;
  stack<pair<bool, Node *>> work;
  for (const auto &sink : obj.getSinks()) {
    work.emplace(true, sink.get());
  }
  // Operate on the stack until empty
  while (!work.empty()) {
    // Pop from the stack
    bool visit = work.top().first;
    auto node = work.top().second;
    work.pop();
    if (indices.count(node)) {
      continue;
    }

    if (visit) {
      work.emplace(false, node);

      // Add the operands to work stack with visit flag set to true
      for (const auto &operand : node->getOperands()) {
        work.emplace(true, operand.get());
      }
    } else {
      auto index = next_index;
      next_index += 1;

      // Add this node to the indices map
      indices[node] = index;

      // Add a new node to the message; set the opcode and add operands
      auto node_msg = msg->add_nodes();
      node_msg->set_op(static_cast<uint32_t>(node->m_op_type));
      for (const auto &operand : node->getOperands()) {
        // Add operands to the current node by saving the indices
        node_msg->add_operands(indices.at(operand.get()));
      }
      // Save the attributes for this node
      node->serializeAttr([&]() { return node_msg->add_attributes(); });
    }
  }

  // Save the input node indices and labels
  for (const auto &entry : obj.m_inputs) {
    auto node_name_msg = msg->add_inputs();
    node_name_msg->set_name(entry.first);
    node_name_msg->set_node(indices.at(entry.second.get()));
  }

  // Save the output node indices and labels
  for (const auto &entry : obj.m_outputs) {
    auto node_name_msg = msg->add_outputs();
    node_name_msg->set_name(entry.first);
    node_name_msg->set_node(indices.at(entry.second.get()));
  }
}

void nodesDeserialize(const msg::DagNodes &msg, vector<NodePtr> &nodes,
                      unique_ptr<iyfc::Dag> &obj) {
  for (auto &node : msg.nodes()) {
    auto op = static_cast<OpType>(node.op());
    if (!isValidOp(op)) {
      throw std::logic_error("Invalid op encountered");
    }

    // Create a new node and set its operands (already loaded)
    nodes.emplace_back(obj->makeNode(op));

    for (auto &operand_idx : node.operands()) {
      nodes.back()->addOperand(nodes.at(operand_idx));
    }

    // Load attributes for this node
    for (auto &attribute : node.attributes()) {
      int attr_ret = nodes.back()->loadAttr(attribute);
      if (0 != attr_ret) {
        throw std::logic_error(" nodesDeserialize Invalid loadAttr");
      }
    }
  }

  // Set the inputs
  for (auto &in : msg.inputs()) {
    obj->m_inputs.emplace(in.name(), nodes.at(in.node()));
  }

  // Set the outputs
  for (auto &out : msg.outputs()) {
    obj->m_outputs.emplace(out.name(), nodes.at(out.node()));
  }
}

void dagCommInfoSerialize(const Dag &obj, msg::DagCommInfo *msg) {
  msg->set_dag_version(IYFC_FORMAT_VERSION);
  msg->set_vec_size(obj.getVecSize());
  msg->set_name(obj.getName());
  // Relevant parameters after algorithm selection
  if (obj.m_alo_decision != nullptr) {
    //
    unique_ptr<msg::Alo> p_alo =
        obj.m_alo_decision->serializeAlo(*(obj.m_serialize_para));

    if (p_alo == nullptr) {
      throw std::logic_error("serializeAlo err");
    }
    msg->mutable_alo()->Swap(p_alo.get());
  }

  msg->set_scale(obj.m_scale);
  // boot
  msg->set_enable_bootstrap(obj.m_enable_bootstrap);
  msg->set_after_reduction_depth(obj.m_after_reduction_depth);
}

void dagCommInfoDeSerialize(const msg::DagCommInfo &msg, Dag *obj) {
  // Ensure serialization version is compatible
  if (msg.dag_version() != IYFC_FORMAT_VERSION) {
    throw std::logic_error("Serialization format version mismatch");
  }
  // set alo_decision
  if (msg.has_alo()) {
    // Construct AloDecision if not already initialized
    if (obj->m_alo_decision == nullptr)
      obj->m_alo_decision = std::make_shared<AloDecision>();

    obj->m_alo_decision->loadAloInfoFromMsg(msg.alo());
  }

  obj->m_enable_bootstrap = msg.enable_bootstrap();
  obj->m_after_reduction_depth = msg.after_reduction_depth();
  obj->m_scale = msg.scale();
}

unique_ptr<msg::Dag> serialize(const Dag &obj) {
  // Create a new program message for serialization
  unique_ptr<msg::Dag> msg = std::make_unique<msg::Dag>();
  dagCommInfoSerialize(obj, msg->mutable_comm_info());
  // Node information is required before serialization
  if (obj.m_serialize_para->need_node_info) {
    uint64_t index = 0;
    nodesSerialize(obj, msg->mutable_dag_nodes(), index);
  }

  // msg->set_final_depth(obj.m_final_depth);
  return msg;
}

// Serialize what needs to be serialized
unique_ptr<iyfc::Dag> deserialize(const msg::Dag &msg) {
  unique_ptr<iyfc::Dag> obj =
      std::make_unique<Dag>(msg.comm_info().name(), msg.comm_info().vec_size());
  dagCommInfoDeSerialize(msg.comm_info(), obj.get());
  // Create a vector of node pointers
  vector<NodePtr> nodes;
  nodesDeserialize(msg.dag_nodes(), nodes, obj);

  return obj;
}

std::unique_ptr<msg::DagGroup> serialize(const DagGroup &obj) {
  unique_ptr<msg::DagGroup> msg = std::make_unique<msg::DagGroup>();
  dagCommInfoSerialize(obj, msg->mutable_comm_info());
  if (obj.m_serialize_para->need_node_info) {
    uint64_t index = 0;

    for (const auto &item : obj.m_name2dag) {
      nodesSerialize(*(item.second), msg->add_dags_nodes(), index);
    }
  }
  return msg;
}
std::unique_ptr<DagGroup> deserialize(const msg::DagGroup &msg) {
  unique_ptr<DagGroup> obj = std::make_unique<DagGroup>(
      msg.comm_info().name(), msg.comm_info().vec_size());
  dagCommInfoDeSerialize(msg.comm_info(), obj.get());
  vector<NodePtr> nodes;

  for (const auto &item : msg.dags_nodes()) {
    unique_ptr<iyfc::Dag> child_obj =
        std::make_unique<Dag>(item.name(), msg.comm_info().vec_size());

    // child node
    nodesDeserialize(item, nodes, child_obj);

    // alo
    child_obj->m_alo_decision = obj->m_alo_decision;
    // group input output
    auto v_inputs = child_obj->getInputs();
    auto v_ouputs = child_obj->getOutputs();
    obj->m_group_inputs.insert(v_inputs.begin(), v_inputs.end());
    obj->m_group_outputs.insert(v_ouputs.begin(), v_ouputs.end());

    obj->m_name2dag[item.name()] = child_obj.release();
    child_obj.reset();
    obj->updateGroupIndex();
  }

  return obj;
}
}  // namespace iyfc
