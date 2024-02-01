/*
 * Dag represents the IR graph formed by nodes generated from expressions,
 * forming a topological structure. DagGroup is a group of graphs that can
 * contain multiple Dags and operate in the same cryptographic environment.
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
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "comm_include.h"
#include "constant_value.h"
#include "expr.h"
#include "node.h"
#include "node_attr.h"
#include "op_type.h"
#include "proto/iyfc.pb.h"

namespace iyfc {
/*
 * Check whether there is a cyclic dependency among expr, node, and dag.
 * Todo: Follow the principle of dependency inversion to handle cyclic
 * dependencies.
 */
using namespace std;
template <typename>
class NodeMapOptional;
template <typename>
class NodeMap;
class NodeMapBase;
class Expr;
class AloDecision;
typedef std::shared_ptr<Node> NodePtr;

/**
 * @class Dag
 * @brief DAG, core class, represents the IR graph composed of nodes generated
 * by expressions. It includes information about all operation nodes, output and
 * input nodes, sinks, sources, etc.
 */
class Dag : public std::enable_shared_from_this<Dag> {
 public:
  /**
   * @brief Dag Constructor
   * @param [in] name Name of the DAG
   * @param [in] vec_size Slot size when using ckks/bfv algorithm, default is
   * 1024
   */
  Dag(std::string name, std::uint64_t vec_size = 1024);

  /**
   * @brief Dag Destructor
   * @details Releases resources
   */
  virtual ~Dag();

  Dag(const Dag &copy) = delete;
  Dag &operator=(const Dag &assign) = delete;
  Dag &operator=(Dag &&assign) = delete;

  /**---------------------------------------------------------------
   * Functions related to building nodes
   * ---------------------------------------------------------------
   */

  /**
   * @brief      Set the input node name
   * @param[in]  name
   * @param[in]  type Data type, default is Cipher
   * @return     Expr
   */
  Expr setInput(const string &name, DataType type = DataType::Cipher);

  /**
   * @brief      Set the output node of the m_dagname
   * @param[in]  name  The name of the output node
   * @param[in]  epxr  The expression for the output node
   * @return     0 on success, an error code otherwise
   */
  int setOutput(const string &name, const Expr &epxr);

  /**
   * @brief      Make a constant node with a 32-bit unsigned integer value
   * @param[in]  u32_value  The 32-bit unsigned integer value
   * @return     NodePtr
   */
  NodePtr makeUint32Const(std::int32_t u32_value);

  /**
   * @brief      Make a constant node with a double value
   * @param[in]  value  Ptr of ConstantValue<double>
   * @return     NodePtr
   */
  NodePtr makeConstant(std::unique_ptr<ConstantValue<double>> value);

  /**
   * @brief      Make a constant node with a 64-bit integer value
   * @param[in]  value  Ptr of ConstantValue<int64_t>
   * @return     NodePtr
   */
  NodePtr makeInt64Constant(std::unique_ptr<ConstantValue<int64_t>> value);

  /**
   * @brief      Make a dense constant value node
   * @param[in]  vec_values  Vector containing double values
   * @return     NodePtr
   */
  NodePtr makeDenseConstant(std::vector<double> vec_values);

  /**
   * @brief      Make a dense constant value node with int64_t values
   * @param[in]  vec_values  Vector containing int64_t values
   * @return     NodePtr
   */
  NodePtr makeInt64DenseConstant(std::vector<int64_t> vec_values);

  /**
   * @brief      Make a dense constant value node with int32_t values
   * @param[in]  vec_values  Vector containing int32_t values
   * @return     NodePtr
   */
  NodePtr makeInt32DenseConstant(std::vector<int32_t> vec_values);

  /**
   * @brief      Make a uniform constant value node
   * @param[in]  value  Pointer to the uniform constant
   * @return     NodePtr
   */
  NodePtr makeUniformConstant(double value);

  /**
   * @brief      Make an input node
   * @param[in]  name  Input value name
   * @param[in]  type  Data type (default: cipher)
   * @return     NodePtr
   */
  NodePtr makeInput(const std::string &name, DataType type = DataType::Cipher);

  /**
   * @brief      Make an output node
   * @param[in]  name  Output name
   * @param[in]  Node  Pointer to the associated node
   * @return     NodePtr
   */
  NodePtr makeOutput(std::string name, const NodePtr &Node);

  /**
   * @brief      Make a left rotation node
   * @param[in]  Node   Pointer to the node
   * @param[in]  slots  Number of slots to rotate
   * @return     NodePtr
   */
  NodePtr makeLeftRotation(const NodePtr &Node, std::int32_t slots);

  /**
   * @brief      Make a right rotation node
   * @param[in]  Node   Pointer to the node
   * @param[in]  slots  Number of slots to rotate
   * @return     NodePtr
   */
  NodePtr makeRightRotation(const NodePtr &Node, std::int32_t slots);

  /**
   * @brief      Make a rescale node
   * @param[in]  Node         Pointer to the node
   * @param[in]  rescale_by   Rescale factor
   * @return     NodePtr
   */
  NodePtr makeRescale(const NodePtr &Node, std::uint32_t rescale_by);

  /**
   * @brief      Set the baseline scale for input and output
   */
  void setScaleRange();

  /**---------------------------------------------------------------
   * Functions related to graph properties
   * ---------------------------------------------------------------
   */
  /**
   * @brief Temporarily collect temporary nodes and release them uniformly
   * later.
   */
  void collectExprNode(NodePtr node);

  /**
   * @brief Release a node.
   * @param[in,out] node Node to be released.
   */
  void freeNode(NodePtr &node);

  /**
   * @brief Get the name of the DAG.
   * @return The name of the DAG.
   */
  virtual std::string getName() const;

  /**
   * @brief Set the name of the DAG.
   * @param[in] newName The new name for the DAG.
   */
  void setName(std::string newName);

  /**
   * @brief Set the security level, default is 128 bits.
   * @param[in] level The security level to set.
   */
  void setSecLevel(int level);

  /**
   * @brief Get the number of slots.
   * @return The number of slots.
   */
  std::uint32_t getVecSize() const;

  /**
   * @brief Set the number of slots.
   * @param[in] vec_size The new number of slots.
   */
  void setVecSize(uint32_t vec_size);

  /**
   * @brief Get the actual data count.
   * @details In scenarios where multiple slots represent one number,
   * such as comparing. After base decomposition, 32 slots represent one number.
   * This function retrieves the number of data to be compared.
   * @return The number of data to be compared.
   */
  std::uint32_t getNumSize() const;

  /**
   * @brief Set the actual number of data to be processed.
   * @param[in] vec_size The new number of data to be processed.
   */
  void setNumSize(uint32_t vec_size);

  /** Make a deep copy of this Dag
  std::unique_ptr<Dag> deepCopy();
  */

  /**
   * @brief Get a string representation of the DAG for visualization.
   */
  std::string toDOT() const;
  /**
   * @brief Print the count of nodes in the graph.
   */
  void printNodeCnt();
  /**
   * @brief Remove a source node from the DAG.
   */
  void eraseSource(Node *);
  /**
   * @brief Remove a sinks node from the graph.
   */
  void eraseSinks(Node *);

  /**
   * @brief Check whether shortint is directly supported, used to determine if
   * the concrete library is in use.
   * @return True if shortint is supported, false otherwise.
   */
  bool supportShortInt();

  /**
   * @brief Set the support for shortint.
   * @param b_su Boolean flag indicating whether shortint is supported.
   */
  void setSupportShortInt(bool b_su);

  /**
   * @brief Get information about the libraries used in the DAG.
   * @return A vector of strings containing information about the libraries.
   */
  vector<string> getLibInfo();

  /**
   * @brief Get the index value for the next node in the DAG.
   * @return The index value for the next node.
   */
  uint64_t getNextNodeIndex() { return m_next_node_index; }

  /**
   * @brief Update the index size of the nodemap to avoid overflow.
   */
  void updateNodeMapIndex();

  /**
   * @brief Set the index value for the next node in the DAG.
   * @param next_node_index The new index value for the next node.
   */
  void setNextNodeIndex(uint64_t next_node_index) {
    m_next_node_index = next_node_index;
    updateNodeMapIndex();
  }

  /**---------------------------------------------------------------
   * DAG Process-Related Functions
   * ---------------------------------------------------------------
   */

  /**
   * @brief Generate the key environment.
   * @return 0 if successful.
   */
  int genKey();

  /**
   * @brief Encrypt the inputs.
   * @param inputs Const Valuation& Plain values.
   * @param replace Whether to replace all existing inputs.
   * @return 0 if encryption is successful.
   */
  int encryptInput(const Valuation &inputs, bool replace);

  /**
   * @brief Execute.
   * @return 0 if execution is successful.
   */
  int executor();

  /**
   * @brief Decrypt the results.
   * @param valuation Valuation& Plain results.
   * @return 0 if execution is successful.
   */
  int getDecryptOutput(Valuation &valuation);

  /**
   * @brief Decrypt the results for Python interface.
   */
  Valuation getDecryptOutputForPython();

  /**---------------------------------------------------------------
   * Serialization Related, Serialization Input and Output Based on the Decided
   * Graph -- Follows the Single Responsibility Principle (SRP) todo
   * ---------------------------------------------------------------
   */

  /**
   * @brief Serialize algorithm-related parameters to a file. Used when handling
   * algorithm parameters separately.
   * @param path Path to the file.
   * @return 0 if successful.
   */
  int saveAloInfoToFile(const std::string &path);
  /**
   * @brief Deserialize algorithm-related parameters from a file.
   */
  int loadAloInfoFromFile(const std::string &path);

  /**
   * @brief Serialize inputs to a file.
   */
  int saveInputToFile(const std::string &path);
  /**
   * @brief Deserialize inputs from a file.
   */
  int loadInputFromFile(const std::string &path);
  /**
   * @brief Serialize output results to a file.
   */
  int saveOutputToFile(const std::string &path);
  /**
   * @brief Deserialize output results from a file.
   */
  int loadOutputFromFile(const std::string &path);

  /**
   * @brief Serialize algorithm-related parameters to a string. Used when
   * handling algorithm parameters separately.
   * @param str Reference to the string where the serialized data will be
   * stored.
   * @return 0 if successful.
   */
  int saveAloInfoToStr(std::string &str);
  /**
   * @brief Deserialize algorithm-related parameters from a string.
   */
  int loadAloInfoFromStr(const std::string &str);
  /**
   * @brief Derialize inputs to a string.
   */
  int saveInputToStr(std::string &str);
  /**
   * @brief Deserialize inputs from a string.
   * @param str Input string for deserialization.
   * @param replace If true, replace the existing inputs.
   * @return 0 if successful.
   */
  int loadInputFromStr(const std::string &str, bool replace = false);
  /**
   * @brief Serialize output results to a string.
   */
  int saveOutputToStr(std::string &str);
  /**
   * @brief Deserialize output results from a string.
   */
  int loadOutputFromStr(const std::string &str);

  /**
   * @brief Serialize additional related information.
   * @param type SERIALIZE_DATA_TYPE Type of data to serialize, currently
   * supports boostrapping_key.
   * @param stream std::ostream& Output stream to store the serialized data.
   * @return 0 if successful.
   */
  int saveByType(SERIALIZE_DATA_TYPE type, std::ostream &stream);
  /**
   * @brief Deserialize additional related information.
   * @param type SERIALIZE_DATA_TYPE Type of data to deserialize, currently
   * supports boostrapping_key.
   * @param stream std::istream& Input stream from which to read the serialized
   * data.
   * @return 0 if successful.
   */
  int loadByType(SERIALIZE_DATA_TYPE type, std::istream &stream);
  /**
   * @brief Parameters related to serialization.
   */
  std::unique_ptr<DagSerializePara> m_serialize_para;

  // todo  Change to private or use snapshot (Memento) pattern

  /**---------------------------------------------------------------
   * Public member variables - Need to be uniformly adjusted to private
   * ---------------------------------------------------------------
   */
  bool m_short_int{false};  // Includes short_int division
  bool m_has_int64{false};
  bool m_has_double{true};         // Default to using CKKS
  bool m_enable_bootstrap{false};  // Whether bootstrapping is needed
  uint32_t m_after_reduction_depth{
      0};  // Multiplication depth after rebalancing
  uint32_t m_scale{DEFAULT_SCALE};
  int m_try_reduce_scale_cnt{1};
  // Decision-related parameters
  std::shared_ptr<AloDecision> m_alo_decision = nullptr;

 public:
  /**---------------------------------------------------------------
   * Virtual functions
   * ---------------------------------------------------------------
   */

  /**
   * @brief Make an operation node.
   * @param[in] op_type OpType of the node.
   * @param[in] operands Vector of NodePtr representing operands.
   * @return NodePtr
   */
  virtual NodePtr makeNode(OpType op_type,
                           const std::vector<iyfc::NodePtr> &operands = {});
  /**
   * @brief      Transpile the decision library algorithm parameters.
   * @return     int 0 if transpilation is successful.
   */
  virtual int doTranspile();
  /**
   * @brief      Get source nodes for traversal.
   */
  virtual std::vector<NodePtr> getSources() const;
  /**
   * @brief      Get sink nodes for reverse traversal.
   */
  virtual std::vector<NodePtr> getSinks() const;
  /**
   * @brief      Get input information.
   */
  virtual const std::unordered_map<std::string, NodePtr> &getInputs() const;
  /**
   * @brief      Get output information.
   */
  virtual const std::unordered_map<std::string, NodePtr> &getOutputs() const;
  /**
   * @brief      Get an input node by name.
   * @param[in]  name Input name.
   * @return     NodePtr
   */
  virtual NodePtr getInput(std::string name) const;
  virtual void updateGroupIndex() { return; }

  void printfSources();
  std::unordered_set<Node *> m_sources{};                // Record source nodes
  std::unordered_set<Node *> m_sinks{};                  // Record sink nodes
  std::unordered_map<std::string, NodePtr> m_outputs{};  // Record output nodes
  std::unordered_map<std::string, NodePtr> m_inputs{};   // Record input nodes
  std::unordered_set<std::string> m_inputnames{};

 private:
  /**---------------------------------------------------------------
   * Private member functions
   * ---------------------------------------------------------------
   */
  /**
   * @brief  Initialize the nodemap for obtaining node attributes.
   */
  void initNodeMap(NodeMapBase &node_map);
  /**
   * @brief Register a nodemap for node attribute usage.
   */
  void registerNodeMap(NodeMapBase *annotation);
  /**
   * @brief Unregister a nodemap.
   */
  void unregisterNodeMap(NodeMapBase *annotation);
  /**
   * @brief  Check whether the DAG contains algorithm decision information.
   */
  void checkNullAlo();

 private:
  /**---------------------------------------------------------------
   * Private member variables
   * ---------------------------------------------------------------
   */
  bool m_init{false};
  // DAG multidepth
  void setInputScale(uint32_t scale);
  void setOutPutRange(uint32_t range);
  /**
   * @brief Allocate an index for a new node in the DAG.
   * @return The allocated node index.
   */
  std::uint64_t allocateIndex();
  std::uint64_t m_node_index_base{0};  // Base index for node allocation.

  std::uint64_t m_min_node_index{std::numeric_limits<uint64_t>::max()};
  std::uint64_t m_next_node_index{0};

  int m_sec_level{128};   // Security level of the DAG.
  std::string m_dagname;  // Name of the DAG.
  std::uint32_t m_vec_size;
  std::uint32_t m_num_size{MAX_CMP_NUM};
  // Used to collect temporary variables of expr expressions after their release
  // to avoid node destruction.
  std::unordered_map<uint64_t, NodePtr> m_exprnode_collect;
  NodePtr m_last_exprnode = nullptr;
  std::vector<NodeMapBase *> m_node_maps;

  friend class Expr;
  friend class Node;
  friend class NodeMapBase;
  friend class AloDecision;
  friend class DagGroup;

  /**
   * @brief Serialize the DAG.
   * @param dag The DAG to be serialized.
   * @return A unique pointer to the serialized DAG message.
   */
  friend std::unique_ptr<msg::Dag> serialize(const Dag &dag);
  /**
   * @brief Deserialize the DAG.
   * @return A unique pointer to the deserialized DAG.
   */
  friend std::unique_ptr<Dag> deserialize(const msg::Dag &msg);
};
// Implementation in the proto directory

std::unique_ptr<Dag> deserialize(const msg::Dag &);

/**
 * @class DagGroup
 * @brief Composite pattern: constructs multiple sub-DAGs
 * @details Multiple independent DAGs are combined into one DagGroup
 * to use the same set of encryption/decryption contexts. Note that
 * the node index needs to be updated. Multiple graphs share the same
 * index pool, so when creating a new subgraph, setNextNodeIndex
 * needs to be called to set the next index for the entire group.
 */
class DagGroup : public Dag {
 public:
  /**
   * @brief DagGroup constructor
   * @param[in] name Name of the DagGroup
   * @param[in] vec_size Number of slots maintained for each subgraph
   */
  DagGroup(const string &name, std::uint64_t vec_size = 1024)
      : Dag(name, vec_size) {}
  virtual ~DagGroup();

  /**
   * @brief Add a subgraph to the DagGroup
   * @param[in] name Name of the subgraph
   * @param[in] dag Pointer to the subgraph
   */
  void addDag(const string &name, DagPtr dag);
  /**
   * @brief Perform library and algorithm parameter decision for the entire
   * group
   * @return 0 if successful, -1 otherwise
   */
  virtual int doTranspile();
  /**
   * @brief Get sources of the group
   */
  virtual std::vector<NodePtr> getSources() const;
  /**
   * @brief Get sinks of the group
   */
  virtual std::vector<NodePtr> getSinks() const;
  /**
   * @brief Get inputs of the group
   */
  virtual const std::unordered_map<std::string, NodePtr> &getInputs() const;
  /**
   * @brief Get outputs of the group
   */
  virtual const std::unordered_map<std::string, NodePtr> &getOutputs() const;

  /**
   * @brief   Create a new node for the group
   * @details Nodes added to the group need to be assigned to a specific
   * sub-DAG.
   * @param [in] op_type Operation type
   * @param [in] operands Vector of iyfc::NodePtr representing operands
   * @return NodePtr representing the newly created node
   */
  virtual NodePtr makeNode(OpType op_type,
                           const std::vector<iyfc::NodePtr> &operands = {});
  /**
   * @brief Update group index information
   */
  virtual void updateGroupIndex();
  virtual NodePtr getInput(std::string name) const;

  /**
   * @brief Get a child DAG by name
   * @param[in] name Name of the child graph
   * @return DagPtr representing the child DAG
   */
  DagPtr getChildDagByName(const std::string &name) {
    if (m_name2dag.find(name) != m_name2dag.end()) {
      return m_name2dag[name];
    } else {
      throw std::logic_error("get child dag err!");
    }
  }

 private:
  void checkInputNames(const std::unordered_set<std::string> &names);
  std::unordered_map<std::string, DagPtr> m_name2dag;
  std::unordered_map<std::string, NodePtr> m_group_outputs;
  std::unordered_map<std::string, NodePtr> m_group_inputs;

  /**
   * @brief  Serialize the DagGroup
   */
  friend std::unique_ptr<msg::DagGroup> serialize(const DagGroup &);
  /**
   * @brief  Deserialize the DagGroup
   */
  friend std::unique_ptr<DagGroup> deserialize(const msg::DagGroup &);
};

std::unique_ptr<DagGroup> deserialize(const msg::DagGroup &);

}  // namespace iyfc
