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
 * @class ConstantHandler
 * @brief Base class for handling constant data types in plaintext computations.
 */
template <typename T>
class ConstantHandler {
 public:
  /**
   * @brief Check if the operation type is Rescale.
   */
  bool isRescaleOp(const OpType &op_code) {
    return (op_code == OpType::Rescale);
  }

  /**
   * @brief Check if the operation type is Multiplication.
   */
  bool isMultiplicationOp(const OpType &op_code) {
    return (op_code == OpType::Mul);
  }

  /**
   * @brief Check if the operation type is Addition or Subtraction.
   */
  bool isAdditionOp(const OpType &op_code) {
    return ((op_code == OpType::Add) || (op_code == OpType::Sub));
  }

 public:
  Dag &dag;

  NodeMapOptional<std::uint32_t> &m_scale;  // scale

  std::vector<T> m_scratch1, m_scratch2;

  /**
   * @brief Constructor for ConstantHandler.
   * @param [in] g DAG
   * @param [in] scale Information about the scales of graph nodes.
   */
  ConstantHandler(Dag &g, NodeMapOptional<std::uint32_t> &scale)
      : dag(g), m_scale(scale) {}

  // Addition expansion.
  virtual void add(NodePtr output, const NodePtr &args1, const NodePtr &args2) {
    warn("in constant base add");
  }

  // Subtraction expansion.
  virtual void sub(NodePtr output, const NodePtr &args1, const NodePtr &args2) {
    warn("in constant base sub");
  }

  // Multiplication expansion.
  virtual void mul(NodePtr output, const NodePtr &args1, const NodePtr &args2) {
    warn("in constant base mul");
  }

  // left rotate expansion
  virtual void leftRotate(NodePtr output, const NodePtr &args1,
                          std::int32_t shift) {
    warn("in constant base leftRotate");
  }

  // right rotate expansion
  virtual void rightRotate(NodePtr output, const NodePtr &args1,
                           std::int32_t shift) {
    warn("in constant base rightRotate");
  }

  virtual void negate(NodePtr output, const NodePtr &args1) {
    warn("in constant base negate");
  }

  /**
   * @brief Overloaded operator() for traversing and processing constant node types, including constant types for + - * operations.
   */
  void operator()(
      NodePtr &node) {  // must only be used with forward pass traversal
    auto &args = node->getOperands();
    if (args.size() == 0) return;  // inputs

    for (auto &arg : args) {
      if (arg->m_op_type != OpType::Constant) return;
    }

    auto op_code = node->m_op_type;
    switch (op_code) {
      case OpType::Add:
        assert(args.size() == 2);
        add(node, args[0], args[1]);
        break;
      case OpType::Sub:
        assert(args.size() == 2);
        sub(node, args[0], args[1]);
        break;
      case OpType::Mul:
        assert(args.size() == 2);
        mul(node, args[0], args[1]);
        break;
      case OpType::RotateLeftConst:
        assert(args.size() == 1);
        leftRotate(node, args[0], node->get<RotationAttr>());
        break;
      case OpType::RotateRightConst:
        assert(args.size() == 1);
        rightRotate(node, args[0], node->get<RotationAttr>());
        break;
      case OpType::Negate:
        assert(args.size() == 1);
        negate(node, args[0]);
        break;
      case OpType::Output:
        [[fallthrough]];
      case OpType::Encode:
        break;
      case OpType::Relinearize:
        [[fallthrough]];
      case OpType::ModSwitch:
        [[fallthrough]];
      case OpType::Rescale:
        warn(
            "Encountered HE specific operation %s in unencrypted "
            "computation \n",
            getOpName(op_code).c_str());
        return;
      default:
        warn("Unhandled op %s\n", getOpName(op_code).c_str());
        return;
    }
  }
};

#define GET_TWO_ARG_EXPAND                                                 \
  std::shared_ptr<ConstantValue<T>> ptr1;                                  \
  std::shared_ptr<ConstantValue<T>> ptr2;                                  \
  if (std::is_same<typename std::decay<T>::type, double>::value) {         \
    ptr1 = args1->get<ConstValueAttr>();                                   \
    ptr2 = args2->get<ConstValueAttr>();                                   \
  } else if (std::is_same<typename std::decay<T>::type, int64_t>::value) { \
    ptr1 = args1->get<ConstValueInt64Attr>();                              \
    ptr2 = args2->get<ConstValueInt64Attr>();                              \
  } else                                                                   \
    return;                                                                \
  auto &input1 = ptr1->expand(m_scratch1, dag.getVecSize());               \
  auto &input2 = ptr2->expand(m_scratch2, dag.getVecSize());               \
  std::vector<T> output_value(input1.size());

#define GET_TWO_DOUBLE_ARG_EXPAND                            \
  std::shared_ptr<ConstantValue<double>> ptr1;               \
  std::shared_ptr<ConstantValue<double>> ptr2;               \
  ptr1 = args1->get<ConstValueAttr>();                       \
  ptr2 = args2->get<ConstValueAttr>();                       \
  auto &input1 = ptr1->expand(m_scratch1, dag.getVecSize()); \
  auto &input2 = ptr2->expand(m_scratch2, dag.getVecSize()); \
  std::vector<double> output_value(input1.size());

#define GET_TWO_INT64_ARG_EXPAND                             \
  std::shared_ptr<ConstantValue<int64_t>> ptr1;              \
  std::shared_ptr<ConstantValue<int64_t>> ptr2;              \
  ptr1 = args1->get<ConstValueInt64Attr>();                  \
  ptr2 = args2->get<ConstValueInt64Attr>();                  \
  auto &input1 = ptr1->expand(m_scratch1, dag.getVecSize()); \
  auto &input2 = ptr2->expand(m_scratch2, dag.getVecSize()); \
  std::vector<int64_t> output_value(input1.size());

#define GET_ONE_AGR_EXPAND                                                 \
  std::shared_ptr<ConstantValue<T>> ptr1;                                  \
  if (std::is_same<typename std::decay<T>::type, double>::value) {         \
    ptr1 = args1->get<ConstValueAttr>();                                   \
  } else if (std::is_same<typename std::decay<T>::type, int64_t>::value) { \
    ptr1 = args1->get<ConstValueInt64Attr>();                              \
  } else                                                                   \
    return;                                                                \
  auto &input1 = ptr1->expand(m_scratch1, dag.getVecSize());

#define GET_ONE_DOUBLE_AGR_EXPAND              \
  std::shared_ptr<ConstantValue<double>> ptr1; \
  ptr1 = args1->get<ConstValueAttr>();         \
  auto &input1 = ptr1->expand(m_scratch1, dag.getVecSize());

#define GET_ONE_INT64_AGR_EXPAND                \
  std::shared_ptr<ConstantValue<int64_t>> ptr1; \
  ptr1 = args1->get<ConstValueInt64Attr>();     \
  auto &input1 = ptr1->expand(m_scratch1, dag.getVecSize());

/**
 * @class ConstantDoubleHandler
 * @brief Handles double-type constant nodes, inherits from ConstantHandler
 * @details In principle, all constant nodes in the same DAG are of the same type. If one is double, all are treated as double.
 */
class ConstantDoubleHandler : public ConstantHandler<double> {
 public:
  /**
   * @brief ConstantDoubleHandler constructor
   * @param [in] g dag
   * @param [in] scale Scale information for DAG nodes
   */
  ConstantDoubleHandler(Dag &g, NodeMapOptional<std::uint32_t> &scale)
      : ConstantHandler<double>(g, scale) {}

  /**
   * @brief Replaces the current node with a double constant node
   * @param[in] node Node to be replaced
   * @param[in] output Values of the constant
   * @param[in] node_scale Scale of the new constant node
   */
  void replaceNodeWithConstant(NodePtr node, const std::vector<double> &output,
                               uint32_t node_scale) {
    // auto constant = dag.makeDenseConstant(output);
    auto constant = node->m_dag->makeDenseConstant(output);
    dag.updateGroupIndex();
    m_scale[constant] = node_scale;
    constant->set<EncodeAtScaleAttr>(m_scale[constant]);
    node->replaceAllUsesWith(constant);
    // assert(node->numUses() == 0);
  }

  /**
   * @brief Double constant node addition operation
   */
  virtual void add(NodePtr output, const NodePtr &args1, const NodePtr &args2) {
    GET_TWO_DOUBLE_ARG_EXPAND
    for (std::uint64_t i = 0; i < output_value.size(); ++i) {
      output_value[i] = input1[i] + input2[i];
    }
    this->replaceNodeWithConstant(output, output_value,
                                  std::max(m_scale[args1], m_scale[args2]));
  }

  /**
   * @brief Double constant node subtraction operation
   */
  virtual void sub(NodePtr output, const NodePtr &args1, const NodePtr &args2) {
    GET_TWO_DOUBLE_ARG_EXPAND
    for (std::uint64_t i = 0; i < output_value.size(); ++i) {
      output_value[i] = input1[i] - input2[i];
    }
    this->replaceNodeWithConstant(output, output_value,
                                  std::max(m_scale[args1], m_scale[args2]));
  }

  /**
   * @brief Double constant node multiplication operation
   */
  virtual void mul(NodePtr output, const NodePtr &args1, const NodePtr &args2) {
    GET_TWO_DOUBLE_ARG_EXPAND
    for (std::uint64_t i = 0; i < output_value.size(); ++i) {
      output_value[i] = input1[i] * input2[i];
    }

    this->replaceNodeWithConstant(output, output_value,
                                  std::max(m_scale[args1], m_scale[args2]));
  }

  /**
   * @brief Double constant node left rotate operation
   */
  virtual void leftRotate(NodePtr output, const NodePtr &args1,
                          std::int32_t shift) {
    GET_ONE_DOUBLE_AGR_EXPAND

    while (shift > 0 && shift >= input1.size()) shift -= input1.size();
    while (shift < 0) shift += input1.size();
    std::vector<double> output_value(input1.size());
    for (std::uint64_t i = 0; i < (output_value.size() - shift); ++i) {
      output_value[i] = input1[i + shift];
    }
    for (std::uint64_t i = 0; i < shift; ++i) {
      output_value[output_value.size() - shift + i] = input1[i];
    }
    this->replaceNodeWithConstant(output, output_value, m_scale[args1]);
  }

  /**
   * @brief Double constant node right rotate operation
   */
  virtual void rightRotate(NodePtr output, const NodePtr &args1,
                           std::int32_t shift) {
    GET_ONE_DOUBLE_AGR_EXPAND
    while (shift > 0 && shift >= input1.size()) shift -= input1.size();
    while (shift < 0) shift += input1.size();
    std::vector<double> output_value(input1.size());
    for (std::uint64_t i = 0; i < (output_value.size() - shift); ++i) {
      output_value[i + shift] = input1[i];
    }
    for (std::uint64_t i = 0; i < shift; ++i) {
      output_value[i] = input1[output_value.size() - shift + i];
    }

    this->replaceNodeWithConstant(output, output_value, m_scale[args1]);
  }

  /**
   * @brief Double constant node negate operation
   */
  virtual void negate(NodePtr output, const NodePtr &args1) {
    GET_ONE_DOUBLE_AGR_EXPAND
    std::vector<double> output_value(input1.size());
    for (std::uint64_t i = 0; i < output_value.size(); ++i) {
      output_value[i] = -input1[i];
    }
    this->replaceNodeWithConstant(output, output_value, m_scale[args1]);
  }
};

/**
 * @class ConstantInt64Handler
 * @brief Handles int64-type constant nodes, inherits from ConstantHandler
 * @details If all constant nodes in the DAG are of type int64, they are processed using this class.
 */
class ConstantInt64Handler : public ConstantHandler<int64_t> {
 public:
  /**
   * @brief ConstantInt64Handler constructor
   * @param [in] g dag
   * @param [in] scale Scale information for DAG nodes
   */
  ConstantInt64Handler(Dag &g, NodeMapOptional<std::uint32_t> &scale)
      : ConstantHandler<int64_t>(g, scale) {}

  /**
   * @brief Replaces the current node with an int64_constant node
   * @param[in] node Node to be replaced
   * @param[in] output Values of the constant
   * @param[in] node_scale Scale of the new constant node
   */
  void replaceNodeWithConstant(NodePtr node, const std::vector<int64_t> &output,
                               uint32_t node_scale) {
    // auto constant = dag.makeInt64DenseConstant(output);
    auto constant = node->m_dag->makeInt64DenseConstant(output);
    dag.updateGroupIndex();
    m_scale[constant] = node_scale;
    constant->set<EncodeAtScaleAttr>(m_scale[constant]);
    node->replaceAllUsesWith(constant);
    // assert(node->numUses() == 0);
  }

  // Addition expansion.
  virtual void add(NodePtr output, const NodePtr &args1, const NodePtr &args2) {
    GET_TWO_INT64_ARG_EXPAND
    for (std::uint64_t i = 0; i < output_value.size(); ++i) {
      output_value[i] = input1[i] + input2[i];
    }
    this->replaceNodeWithConstant(output, output_value,
                                  std::max(m_scale[args1], m_scale[args2]));
  }
  // Subtraction expansion.
  virtual void sub(NodePtr output, const NodePtr &args1, const NodePtr &args2) {
    GET_TWO_INT64_ARG_EXPAND
    for (std::uint64_t i = 0; i < output_value.size(); ++i) {
      output_value[i] = input1[i] - input2[i];
    }
    this->replaceNodeWithConstant(output, output_value,
                                  std::max(m_scale[args1], m_scale[args2]));
  }
  // ultiplication expansion.
  virtual void mul(NodePtr output, const NodePtr &args1, const NodePtr &args2) {
    GET_TWO_INT64_ARG_EXPAND
    for (std::uint64_t i = 0; i < output_value.size(); ++i) {
      output_value[i] = input1[i] * input2[i];
    }
    replaceNodeWithConstant(output, output_value,
                            std::max(m_scale[args1], m_scale[args2]));
  }
  // left rotate expansion
  virtual void leftRotate(NodePtr output, const NodePtr &args1,
                          std::int32_t shift) {
    GET_ONE_INT64_AGR_EXPAND

    while (shift > 0 && shift >= input1.size()) shift -= input1.size();
    while (shift < 0) shift += input1.size();
    std::vector<int64_t> output_value(input1.size());
    for (std::uint64_t i = 0; i < (output_value.size() - shift); ++i) {
      output_value[i] = input1[i + shift];
    }
    for (std::uint64_t i = 0; i < shift; ++i) {
      output_value[output_value.size() - shift + i] = input1[i];
    }
    this->replaceNodeWithConstant(output, output_value, m_scale[args1]);
  }
  // right rotate expansion
  virtual void rightRotate(NodePtr output, const NodePtr &args1,
                           std::int32_t shift) {
    GET_ONE_INT64_AGR_EXPAND
    while (shift > 0 && shift >= input1.size()) shift -= input1.size();
    while (shift < 0) shift += input1.size();
    std::vector<int64_t> output_value(input1.size());
    for (std::uint64_t i = 0; i < (output_value.size() - shift); ++i) {
      output_value[i + shift] = input1[i];
    }
    for (std::uint64_t i = 0; i < shift; ++i) {
      output_value[i] = input1[output_value.size() - shift + i];
    }

    this->replaceNodeWithConstant(output, output_value, m_scale[args1]);
  }
  virtual void negate(NodePtr output, const NodePtr &args1) {
    GET_ONE_INT64_AGR_EXPAND
    std::vector<int64_t> output_value(input1.size());
    for (std::uint64_t i = 0; i < output_value.size(); ++i) {
      output_value[i] = -input1[i];
    }
    this->replaceNodeWithConstant(output, output_value, m_scale[args1]);
  }
};

}  // namespace iyfc
