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
#include "iyfc_dag.h"
#include <fstream>
#include <sstream>
#include "decision/alo_decision.h"
#include "err_code.h"

using namespace std;

namespace iyfc {

vector<NodePtr> toNodePtrs(const unordered_set<Node *> &nodes) {
  vector<NodePtr> nodeptrs;
  nodeptrs.reserve(nodes.size());
  for (auto &Node : nodes) {
    nodeptrs.emplace_back(Node->shared_from_this());
  }
  return nodeptrs;
}

Dag::Dag(std::string name, std::uint64_t vec_size)
    : m_next_node_index(0), m_dagname(name), m_vec_size(vec_size) {
  if (m_vec_size == 0) {
    throw std::logic_error("vector size must be non-zero");
  }

  if ((m_vec_size & (m_vec_size - 1)) != 0) {
    throw std::logic_error("vector size must be a power-of-two");
  }
  m_serialize_para = std::make_unique<DagSerializePara>(false, false, false,
                                                        false, false, false);
  m_init = true;
}

Dag::~Dag() {
  m_exprnode_collect.clear();
  m_outputs.clear();
  m_inputs.clear();
  m_sources.clear();
  m_sinks.clear();
  m_node_maps.clear();
  m_init = false;
}

void Dag::printNodeCnt() {
  for (auto &&item : m_exprnode_collect) {
    printf("node cnt item.use_count %lu, index %lu \n", item.second.use_count(),
           item.first);
  }
}

const std::unordered_map<std::string, NodePtr> &Dag::getInputs() const {
  return m_inputs;
}

const std::unordered_map<std::string, NodePtr> &Dag::getOutputs() const {
  return m_outputs;
}

// dag m_dagname size
std::string Dag::getName() const { return m_dagname; }
void Dag::setName(std::string newName) { m_dagname = newName; }
std::uint32_t Dag::getVecSize() const { return m_vec_size; }
void Dag::setVecSize(uint32_t vec_size) { m_vec_size = vec_size; }
std::uint32_t Dag::getNumSize() const { return m_num_size; }
void Dag::setNumSize(uint32_t num_size) { m_num_size = num_size; }

vector<NodePtr> Dag::getSources() const { return toNodePtrs(this->m_sources); }

vector<NodePtr> Dag::getSinks() const { return toNodePtrs(this->m_sinks); }

// std::unique_ptr<Dag> Dag::deepCopy() {
//   auto new_dag = std::make_unique<Dag>(getName(), getVecSize());
//   /*
//   NodeMap<NodePtr> old2new(*this);
//   DagTraversal traversal(*this);

//   traversal.forwardPass([&](NodePtr &node) {
//     auto new_node = new_dag->makeNode(node->m_op_type);
//     old2new[node] = new_node;
//     new_node->assignAttrFrom(*node);
//     for (auto &operand : node->getOperands()) {
//       new_node->addOperand(old2new[operand]);
//     }
//   });

//   for (auto &entry : m_inputs) {
//     new_dag->m_inputs[entry.first] = old2new[entry.second];
//   }
//   for (auto &entry : m_outputs) {
//     new_dag->m_outputs[entry.first] = old2new[entry.second];
//   }
//   */
//   return new_dag;
// }

//
NodePtr Dag::makeNode(OpType op_type, const std::vector<NodePtr> &operands) {
  auto node = std::make_shared<Node>(op_type, this);
  if (operands.size() > 0) {
    node->setOperands(operands);
  }
  if (op_type == OpType::Div) m_short_int = true;
  if (op_type != OpType::Input && op_type != OpType::Output)
    collectExprNode(node);
  return node;
}

NodePtr Dag::makeUint32Const(std::int32_t u32_value) {
  auto node = makeNode(OpType::U32Constant);
  node->set<Uint32ConstAttr>(u32_value);
  return node;
}

NodePtr Dag::makeConstant(std::unique_ptr<ConstantValue<double>> value) {
  m_has_double = true;
  auto node = makeNode(OpType::Constant);

  node->set<ConstValueAttr>(std::move(value));
  // node->set<TypeAttr>(DataType::Raw);
  return node;
}

NodePtr Dag::makeInt64Constant(std::unique_ptr<ConstantValue<int64_t>> value) {
  m_has_int64 = true;
  auto node = makeNode(OpType::Constant);
  node->set<ConstValueInt64Attr>(std::move(value));
  // node->set<TypeAttr>(DataType::Raw);
  return node;
}

NodePtr Dag::makeDenseConstant(std::vector<double> vec_values) {
  return makeConstant(
      std::make_unique<DenseConstantValue<double>>(m_vec_size, vec_values));
}

NodePtr Dag::makeInt64DenseConstant(std::vector<int64_t> vec_values) {
  return makeInt64Constant(
      std::make_unique<DenseConstantValue<int64_t>>(m_vec_size, vec_values));
}

NodePtr Dag::makeInt32DenseConstant(std::vector<int32_t> vec_values) {
  std::vector<int64_t> tmp_values(vec_values.size());
  std::transform(
      vec_values.begin(), vec_values.end(), tmp_values.begin(),
      [](const int int_value) { return static_cast<int64_t>(int_value); });

  return makeInt64Constant(
      std::make_unique<DenseConstantValue<int64_t>>(m_vec_size, tmp_values));
}

NodePtr Dag::makeUniformConstant(double value) {
  return makeDenseConstant({value});
}

NodePtr Dag::makeInput(const std::string &name, DataType type) {
  auto node_ptr = makeNode(OpType::Input);
  node_ptr->set<TypeAttr>(type);
  m_inputs.emplace(name, node_ptr);
  return node_ptr;
}

NodePtr Dag::makeOutput(std::string name, const NodePtr &Node) {
  auto output = makeNode(OpType::Output, {Node});
  m_outputs.emplace(name, output);
  return output;
}

NodePtr Dag::makeLeftRotation(const NodePtr &Node, std::int32_t slots) {
  auto rotation = makeNode(OpType::RotateLeftConst, {Node});
  rotation->set<RotationAttr>(slots);
  return rotation;
}

NodePtr Dag::makeRightRotation(const NodePtr &Node, std::int32_t slots) {
  auto rotation = makeNode(OpType::RotateRightConst, {Node});
  rotation->set<RotationAttr>(slots);
  return rotation;
}

NodePtr Dag::makeRescale(const NodePtr &Node, std::uint32_t rescale_by) {
  auto rescale = makeNode(OpType::Rescale, {Node});
  rescale->set<RescaleDivisorAttr>(rescale_by);
  return rescale;
}

Expr Dag::setInput(const string &name, DataType type) {
  // return Expr(shared_from_this(), makeInput(name));
  m_inputnames.insert(name);
  return Expr(this, makeInput(name, type));
}

void Dag::setInputScale(uint32_t scale) {
  for (auto &source : getSources()) {
    source->set<EncodeAtScaleAttr>(scale);
  }
}

void Dag::setOutPutRange(uint32_t range) {
  for (auto &entry : getOutputs()) {
    entry.second->set<RangeAttr>(range);
  }
}

NodePtr Dag::getInput(std::string name) const { return m_inputs.at(name); }

int Dag::setOutput(const string &name, const Expr &epxr) {
  if (epxr.m_nodeptr == nullptr) {
    throw std::logic_error("no expr input");
  }
  makeOutput(name, epxr.m_nodeptr);
  return 0;
}

void Dag::updateNodeMapIndex() {
  for (NodeMapBase *node_map : m_node_maps) {
    node_map->resize(m_next_node_index);
  }
}

uint64_t Dag::allocateIndex() {
  uint64_t index = m_next_node_index++;
  m_min_node_index = std::min(m_min_node_index, index);
  updateNodeMapIndex();
  return index;
}

void Dag::initNodeMap(NodeMapBase &node_map) {
  node_map.resize(m_next_node_index);
}

void Dag::registerNodeMap(NodeMapBase *node_map) {
  m_node_maps.emplace_back(node_map);
}

void Dag::unregisterNodeMap(NodeMapBase *node_map) {
  auto iter = find(m_node_maps.begin(), m_node_maps.end(), node_map);
  if (iter == m_node_maps.end()) {
    throw std::logic_error("NodeMap to unregister not found");
  } else {
    m_node_maps.erase(iter);
  }
}

void Dag::setSupportShortInt(bool b_su) { m_short_int = b_su; }
bool Dag::supportShortInt() { return m_short_int; }

void Dag::collectExprNode(NodePtr node) {
  if (m_exprnode_collect.find(node->m_index) == m_exprnode_collect.end()) {
    m_exprnode_collect.emplace(node->m_index, node);
  }
}

void Dag::freeNode(NodePtr &node) {}
void Dag::setSecLevel(int level) { m_sec_level = level; }

template <class Attr>
void dumpAttr(stringstream &s, Node *Node, std::string label) {
  if (Node->has<Attr>()) {
    s << ", " << label << "=" << Node->get<Attr>();
  }
}

// Print an attribute in DOT format as a box outside the Node
template <class Attr>
void toDOTAttrAsNode(stringstream &s, Node *node, std::string label) {
  if (node->has<Attr>()) {
    s << "t" << node->m_index << "_" << getAttrName(Attr::key)
      << " [shape=box label=\"" << label << "=" << node->get<Attr>()
      << "\"];\n";
    s << "t" << node->m_index << "_" << getAttrName(Attr::key) << " -> t"
      << node->m_index << ";\n";
  }
}
/*
string Dag::dump(NodeMapOptional<std::uint32_t> &scales,
                     NodeMap<iyfc::DataType> &types,
                     NodeMap<std::uint32_t> &level) const {
  // TODO: switch to use a non-parallel generic traversal
  stringstream s;
  s << getName() << "(){\n";

  // Add all terms in topologically sorted order
  uint64_t next_index = 0;
  unordered_map<Node *, uint64_t> map_indices;
  stack<pair<bool, Node *>> st_work;
  for (const auto &sink : getSinks()) {
    st_work.emplace(true, sink.get());
  }
  while (!st_work.empty()) {
    bool visit = st_work.top().first;
    auto node = st_work.top().second;
    st_work.pop();
    if (map_indices.count(node)) {
      continue;
    }
    if (visit) {
      st_work.emplace(false, node);
      for (const auto &operand : node->getOperands()) {
        st_work.emplace(true, operand.get());
      }
    } else {
      auto index = next_index;
      next_index += 1;
      map_indices[node] = index;
      s << "t" << node->m_index << " = " << getOpName(node->m_op_type);
      if (node->has<RescaleDivisorAttr>()) {
        s << "(" << node->get<RescaleDivisorAttr>() << ")";
      }
      if (node->has<RotationAttr>()) {
        s << "(" << node->get<RotationAttr>() << ")";
      }
      if (node->has<TypeAttr>()) {
        s << ":" << getTypeName(node->get<TypeAttr>());
      }
      for (int i = 0; i < node->numOperands(); ++i) {
        s << " t" << node->operandAt(i)->m_index;
      }
      dumpAttr<RangeAttr>(s, node, "range");
      dumpAttr<EncodeAtLevelAttr>(s, node, "level");
      if (types[*node] == DataType::Cipher)
        s << ", "
          << "s"
          << "=" << scales[*node] << ", t=cipher ";
      else if (types[*node] == DataType::Raw)
        s << ", "
          << "s"
          << "=" << scales[*node] << ", t=raw ";
      else
        s << ", "
          << "s"
          << "=" << scales[*node] << ", t=plain ";
      s << "\n";
      // ConstantValue<double> TODO: printing constant values for simple cases
    }
  }

  s << "}\n";
  return s.str();
}
*/

string Dag::toDOT() const {
  // TODO: switch to use a non-parallel generic traversal
  stringstream s;

  // s << "digraph \"" << getName() << "sinks_size : " << m_sinks.size()
  s << "{\n";

  // Add all terms in topologically sorted order
  uint64_t next_index = 0;
  unordered_map<Node *, uint64_t> map_indices;
  stack<pair<bool, Node *>> st_work;
  for (const auto &sink : getSinks()) {
    st_work.emplace(true, sink.get());
  }
  while (!st_work.empty()) {
    bool visit = st_work.top().first;
    auto node = st_work.top().second;
    st_work.pop();
    if (map_indices.count(node)) {
      continue;
    }
    if (visit) {
      st_work.emplace(false, node);
      for (const auto &operand : node->getOperands()) {
        st_work.emplace(true, operand.get());
      }
    } else {
      auto index = next_index;
      next_index += 1;
      map_indices[node] = index;

      // Operands are guaranteed to have been added
      s << "t" << node->m_index << " [label=\"" << getOpName(node->m_op_type);
      if (node->has<RescaleDivisorAttr>()) {
        s << "(" << node->get<RescaleDivisorAttr>() << ")";
      }
      if (node->has<RotationAttr>()) {
        s << "(" << node->get<RotationAttr>() << ")";
      }
      if (node->has<TypeAttr>()) {
        s << " : " << getTypeName(node->get<TypeAttr>());
      }
      s << "\"";  // End label
      s << "];\n";
      for (int i = 0; i < node->numOperands(); ++i) {
        s << "t" << node->operandAt(i)->m_index << " -> t" << node->m_index
          << " [label=\"" << i << "\"];\n";
      }
      toDOTAttrAsNode<RangeAttr>(s, node, "range");
      toDOTAttrAsNode<EncodeAtScaleAttr>(s, node, "scale");
      toDOTAttrAsNode<EncodeAtLevelAttr>(s, node, "level");
      // ConstantValue<double> TODO: printing constant values for simple cases
    }
  }

  s << "}\n";

  return s.str();
}

void Dag::setScaleRange() {
  setInputScale(m_scale);
  setOutPutRange(m_scale);
}

void Dag::checkNullAlo() {
  if (m_alo_decision == nullptr) {
    throw std::logic_error("alo_decision_ptr null");
    throw std::logic_error("alo_decision_ptr null");
  }
}

void Dag::eraseSource(Node *node) { m_sources.erase(node); }
void Dag::eraseSinks(Node *node) { m_sinks.erase(node); }

int Dag::doTranspile() {
  m_alo_decision = std::make_shared<AloDecision>();
  // Decide algorithm
  return m_alo_decision->deLibAndAlo(*this);
}

int Dag::genKey() { return m_alo_decision->genKeys(*this); }

int Dag::encryptInput(const Valuation &inputs, bool replace) {
  checkNullAlo();
  return m_alo_decision->encryptInput(inputs, replace);
}

int Dag::executor() {
  checkNullAlo();
  return m_alo_decision->executor(*this);
}

int Dag::getDecryptOutput(Valuation &valuation) {
  checkNullAlo();
  return m_alo_decision->getDecryptOutput(valuation);
}

int Dag::saveAloInfoToFile(const std::string &path) {
  if (m_alo_decision == nullptr) {
    // Decision execution separation
    m_alo_decision = std::make_shared<AloDecision>();
  }
  std::ofstream out(path);
  if (out.fail()) {
    throw std::logic_error("saveAloInfoToFile Could not open file");
  }
  auto p_msg = m_alo_decision->serializeAlo(*m_serialize_para);
  if (p_msg != nullptr && !p_msg->SerializeToOstream(&out)) {
    throw std::logic_error("Could not serialize message");
  }
  return 0;
}

int Dag::loadAloInfoFromFile(const std::string &path) {
  if (m_alo_decision == nullptr) {
    // Decision execution separation
    m_alo_decision = std::make_shared<AloDecision>();
  }

  ifstream in(path);
  if (in.fail()) {
    throw std::logic_error("loadAloInfoFromFile Could not open file");
  }
  msg::Alo msg;
  if (msg.ParseFromIstream(&in)) {
    return m_alo_decision->loadAloInfoFromMsg(msg);
  } else {
    throw std::logic_error("Could not parse message");
  }
}

int Dag::saveInputToFile(const std::string &path) {
  checkNullAlo();
  std::ofstream out(path);
  if (out.fail()) {
    throw std::logic_error("saveInputToFile Could not open file");
  }
  auto p_msg = m_alo_decision->serializeInput();
  if (p_msg != nullptr && !p_msg->SerializeToOstream(&out)) {
    throw std::logic_error("Could not serialize message");
  }
  return 0;
}

int Dag::loadInputFromFile(const std::string &path) {
  checkNullAlo();
  ifstream in(path);
  if (in.fail()) {
    throw std::logic_error("loadInputFromFile Could not open file");
  }
  msg::Input msg;
  if (msg.ParseFromIstream(&in)) {
    return m_alo_decision->loadInputFromMsg(msg);
  } else {
    throw std::logic_error("loadInputFromFile Could not parse message");
  }
}

int Dag::saveOutputToFile(const std::string &path) {
  checkNullAlo();
  std::ofstream out(path);
  if (out.fail()) {
    throw std::logic_error("saveOutputToFile Could not open file");
    return SAVE_OUTPUT_FILE_NOT_OPEN;
  }
  auto p_msg = m_alo_decision->serializeOutput();
  if (p_msg != nullptr && !p_msg->SerializeToOstream(&out)) {
    throw std::logic_error("saveOutputToFileCould not serialize message");
  }
  return 0;
}

int Dag::loadOutputFromFile(const std::string &path) {
  checkNullAlo();
  ifstream in(path);
  if (in.fail()) {
    throw std::logic_error("loadOutputFromFile Could not open file");
  }
  msg::Output msg;
  if (msg.ParseFromIstream(&in)) {
    return m_alo_decision->loadOutputFromMsg(msg);
  } else {
    throw std::logic_error("loadOutputFromFile Could not parse message");
  }
}

int Dag::saveAloInfoToStr(std::string &str) {
  if (m_alo_decision == nullptr) {
    // Decision execution separation
    m_alo_decision = std::make_shared<AloDecision>();
  }
  auto p_msg = m_alo_decision->serializeAlo(*m_serialize_para);
  if (p_msg == nullptr) {
    throw std::logic_error("ser alo info  nullptr \n");
  }

  if (p_msg != nullptr && !p_msg->SerializeToString(&str)) {
    throw std::logic_error("saveAloInfoToStr Could not serialize message");
  }
  return 0;
}

int Dag::loadAloInfoFromStr(const std::string &str) {
  if (m_alo_decision == nullptr) {
    // Decision execution separation
    m_alo_decision = std::make_shared<AloDecision>();
  }
  msg::Alo msg;
  if (msg.ParseFromString(str)) {
    return m_alo_decision->loadAloInfoFromMsg(msg);
  } else {
    throw std::logic_error("loadAloInfoFromStr Could not parse message");
  }
}

int Dag::saveInputToStr(std::string &str) {
  checkNullAlo();
  auto p_msg = m_alo_decision->serializeInput();

  if (p_msg != nullptr && !p_msg->SerializeToString(&str)) {
    throw std::logic_error("saveInputToStr Could not serialize message");
  }
  return 0;
}

int Dag::loadInputFromStr(const std::string &str, bool replace) {
  checkNullAlo();
  msg::Input msg;
  if (msg.ParseFromString(str)) {
    return m_alo_decision->loadInputFromMsg(msg, replace);
  } else {
    throw std::logic_error(" loadInputFromStr Could not parse message");
  }
}

int Dag::saveOutputToStr(std::string &str) {
  checkNullAlo();
  auto p_msg = m_alo_decision->serializeOutput();
  if (p_msg != nullptr && !p_msg->SerializeToString(&str)) {
    throw std::logic_error("saveOutputToStr Could not serialize message");
  }
  return 0;
}
int Dag::loadOutputFromStr(const std::string &str) {
  checkNullAlo();
  msg::Output msg;
  if (msg.ParseFromString(str)) {
    return m_alo_decision->loadOutputFromMsg(msg);
  } else {
    throw std::logic_error("loadOutputFromStr Could not parse message");
  }
}

int Dag::saveByType(SERIALIZE_DATA_TYPE type, std::ostream &stream) {
  checkNullAlo();
  return m_alo_decision->serializeByType(type, stream);
}

int Dag::loadByType(SERIALIZE_DATA_TYPE type, std::istream &stream) {
  checkNullAlo();
  return m_alo_decision->deserializeByType(type, stream);
}

// To match Python conventions, return the output as a parameter.
Valuation Dag::getDecryptOutputForPython() {
  Valuation valuation;
  if (m_alo_decision == nullptr) {
    throw std::logic_error("err getDecryptOutput alo_decision_ptr null");
  }
  m_alo_decision->getDecryptOutput(valuation);
  return valuation;
}

vector<string> Dag::getLibInfo() {
  if (m_alo_decision != nullptr)
    return m_alo_decision->getLibs();
  else
    return {};
}

DagGroup::~DagGroup() {
  m_name2dag.clear();
  m_group_outputs.clear();
  m_group_inputs.clear();
}

void DagGroup::addDag(const string &name, DagPtr dag) {
  // Check if there is any intersection in indices
  if (dag->m_min_node_index < m_next_node_index) {
    throw std::logic_error(
        "group add dag! new child dag min_node_index must bigger than group "
        "next_node_index ! setNextNodeIndex before build a new child dag!");
  }
  // checkInputNames(dag->m_inputnames);

  if (m_name2dag.find(name) != m_name2dag.end()) {
    throw std::logic_error("add same name dag err!");
  }
  auto v_inputs = dag->getInputs();
  auto v_ouputs = dag->getOutputs();
  if (v_inputs.size() == 0 || v_ouputs.size() == 0) {
    throw std::logic_error("group_dag can not add empty inputs or outputs dag");
  }
  m_group_inputs.insert(v_inputs.begin(), v_inputs.end());
  m_group_outputs.insert(v_ouputs.begin(), v_ouputs.end());
  this->m_vec_size = std::max(this->m_vec_size, dag->m_vec_size);

  m_name2dag[name] = dag;
  // Update all scales to maintain consistency
  if (dag->m_scale < m_scale) {
    m_scale = dag->m_scale;
    for (auto &item : m_name2dag) {
      item.second->m_scale = m_scale;
    }
  }
  // Update total index count
  updateGroupIndex();
}

NodePtr DagGroup::makeNode(OpType op_type,
                           const std::vector<NodePtr> &operands) {
  // Group adds new nodes, assign them to specific subgraphs
  if (operands.size() > 0 && operands[0]->m_dag != this) {
    auto node = operands[0]->m_dag->makeNode(op_type, operands);
    updateGroupIndex();
    return node;
  } else {
    auto node = Dag::makeNode(op_type, operands);
    updateGroupIndex();
    return node;
  }
}

int DagGroup::doTranspile() {
  m_alo_decision = std::make_shared<AloDecision>();
  // Determine algorithm
  if (0 == m_alo_decision->deGroupLibAndAlo(*this, m_name2dag)) {
    for (auto &item : m_name2dag) item.second->m_alo_decision = m_alo_decision;
  } else {
    throw std::logic_error(" group transpile err");
  }
  return 0;
};

vector<NodePtr> DagGroup::getSources() const {
  // 1. The group may already have new nodes
  vector<NodePtr> nodeptrs = Dag::getSources();

  // 2. Nodes of the sub-DAGs
  for (auto &item : m_name2dag) {
    auto v = toNodePtrs(item.second->m_sources);
    std::move(v.begin(), v.end(), std::back_inserter(nodeptrs));
  }
  return nodeptrs;
}

vector<NodePtr> DagGroup::getSinks() const {
  // 1. The group may already have newly added nodes
  vector<NodePtr> nodeptrs = Dag::getSinks();
  // 2. Nodes from sub DAGs
  for (auto &item : m_name2dag) {
    auto v = toNodePtrs(item.second->m_sinks);
    std::move(v.begin(), v.end(), std::back_inserter(nodeptrs));
  }
  return nodeptrs;
}

const std::unordered_map<std::string, NodePtr> &DagGroup::getInputs() const {
  LOG(LOGLEVEL::Debug, "update group getInputs size %zu",
      m_group_inputs.size());
  return m_group_inputs;
}

const std::unordered_map<std::string, NodePtr> &DagGroup::getOutputs() const {
  return m_group_outputs;
}

NodePtr DagGroup::getInput(std::string name) const {
  return m_group_inputs.at(name);
}

void DagGroup::updateGroupIndex() {
  uint64_t total_index = m_next_node_index;
  // Find the maximum index among all shared indices and update
  for (auto &item : m_name2dag) {
    total_index = std::max(total_index, item.second->getNextNodeIndex());
  }
  for (auto &item : m_name2dag) {
    if (total_index > item.second->getNextNodeIndex()) {
      // Update sub DAGs index
      item.second->setNextNodeIndex(total_index);
    }
  }
  // Update the total index of the parent
  if (total_index > m_next_node_index) setNextNodeIndex(total_index);
}

void DagGroup::checkInputNames(const std::unordered_set<std::string> &names) {
  std::unordered_set<std::string> inter;
  std::set_intersection(m_inputnames.cbegin(), m_inputnames.cend(),
                        names.cbegin(), names.cend(),
                        inserter(inter, inter.begin()));
  if (inter.empty()) {
    std::unordered_set<std::string> tmp_union;
    std::set_union(m_inputnames.cbegin(), m_inputnames.cend(), names.cbegin(),
                   names.cend(), inserter(tmp_union, tmp_union.begin()));
    std::swap(m_inputnames, tmp_union);
  } else {
    throw std::logic_error(
        "group add dag err ! input names must be different!");
  }
}
}  // namespace iyfc
