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

#include <algorithm>
#include <functional>
#include <numeric>
#include <variant>
#include <vector>

#include "comm_include.h"
#include "dag/constant_value.h"
#include "dag/iyfc_dag.h"
#include "dag/node_map.h"
#include "err_code.h"
#include "openfhe.h"
#include "openfhe_valuation.h"
#include "util/logging.h"
#include "util/overloaded.h"
#include "util/timer.h"

using namespace lbcrypto;

namespace iyfc {

#ifndef OPENFHE_EXE_CHECK_ERROR
#define OPENFHE_EXE_CHECK_ERROR(func_ret, err_msg) \
  if (!(func_ret)) {                               \
    warn("openfhe dag exe err: %s", err_msg);      \
    m_has_err = true;                              \
    return;                                        \
  }
#endif

//

#define GET_CONSTANT_NODE_ATTR                                             \
  std::shared_ptr<ConstantValue<T>> ptr1;                                  \
  if (std::is_same<typename std::decay<T>::type, double>::value) {         \
    ptr1 = node->get<ConstValueAttr>();                                    \
  } else if (std::is_same<typename std::decay<T>::type, int64_t>::value) { \
    ptr1 = node->get<ConstValueInt64Attr>();                               \
  } else                                                                   \
    return;

/**
 * @class OpenfheExecutor execution class
 * @brief Traverse and execute graph nodes
 */
template <typename T>
class OpenfheExecutor {
 public:
  using RuntimeValue = std::variant<OpenFheCiphertext, OpenFhePlaintext,
                                    std::vector<T>>;  

  Dag &dag;
  OpenFheContext context;  // context

  NodeMapOptional<RuntimeValue> m_objects;
  bool m_has_err{false};
  uint32_t m_final_depth;  // Multiplication depth
  std::vector<T> temp_vec;

  bool isCipher(const NodePtr &t) {
    return std::holds_alternative<OpenFheCiphertext>(m_objects.at(t));
  }
  bool isPlain(const NodePtr &t) {
    return std::holds_alternative<OpenFhePlaintext>(m_objects.at(t));
  }
  bool isRaw(const NodePtr &t) {
    return std::holds_alternative<std::vector<T>>(m_objects.at(t));
  }

  /**
   * @brief Right shift of the original data vector
   */
  void rightRotateRaw(std::vector<T> &out, const NodePtr &args1,
                      std::int32_t shift) {
    auto &in = std::get<std::vector<T>>(m_objects.at(args1));

    while (shift > 0 && shift >= in.size()) shift -= in.size();
    while (shift < 0) shift += in.size();

    out.clear();
    out.reserve(in.size());
    copy_n(in.cend() - shift, shift, back_inserter(out));
    copy_n(in.cbegin(), in.size() - shift, back_inserter(out));
  }

  /**
   * @brief Left shift of the original data vector
   */
  void leftRotateRaw(std::vector<T> &out, const NodePtr &args1,
                     std::int32_t shift) {
    auto &in = std::get<std::vector<T>>(m_objects.at(args1));

    while (shift > 0 && shift >= in.size()) shift -= in.size();
    while (shift < 0) shift += in.size();

    out.clear();
    out.reserve(in.size());
    copy_n(in.cbegin() + shift, in.size() - shift, back_inserter(out));
    copy_n(in.cbegin(), shift, back_inserter(out));
  }

  /**
   * @brief Processing functions for primitive data types in nodes
   */
  template <class OpType>
  void binOpRaw(std::vector<T> &out, const NodePtr &args1,
                const NodePtr &args2) {
    auto &in1 = std::get<std::vector<T>>(m_objects.at(args1));
    auto &in2 = std::get<std::vector<T>>(m_objects.at(args2));

    OPENFHE_EXE_CHECK_ERROR(in1.size() == in2.size(),
                            "binopraw arg1.size 1= arg2.size");

    out.clear();
    out.reserve(in1.size());
    transform(in1.cbegin(), in1.cend(), in2.cbegin(), back_inserter(out),
              OpType());
  }

  /**
   * @brief Negate the raw node data
   */
  void negateRaw(std::vector<T> &out, const NodePtr &args1) {
    auto &in = std::get<std::vector<T>>(m_objects.at(args1));

    out.clear();
    out.reserve(in.size());
    transform(in.cbegin(), in.cend(), back_inserter(out), std::negate<T>());
  }

  /**
   * @brief add
   * @param [out] out The result of the addition operation is of type OpenFheCiphertext
   * @param [in] args1 Left operand node
   * @param [in] args2 Right operand node
   */
  void add(OpenFheCiphertext &output, const NodePtr &args1,
           const NodePtr &args2) {
    if (!isCipher(args1)) {
      OPENFHE_EXE_CHECK_ERROR(isCipher(args2),
                              " add: arg1 or arg2 must be cipher ");

      add(output, args2, args1);
      return;
    }
    OpenFheCiphertext &input1 =
        std::get<OpenFheCiphertext>(m_objects.at(args1));
    std::visit(Overloaded{[&](const OpenFheCiphertext &input2) {
                            output = context->EvalAdd(input1, input2);
                          },
                          [&](const OpenFhePlaintext &input2) {
                            output = context->EvalAdd(input1, input2);
                          },
                          [&](const std::vector<T> &input2) {
                            warn("Unsupported operation encountered");
                            return;
                          }},
               m_objects.at(args2));
  }

  /**
   * @brief sub
   * @param [out] out The result of the subtraction operation is of type OpenFheCiphertext
   * @param [in] args1 Left operand node
   * @param [in] args2 Right operand node
   */
  void sub(OpenFheCiphertext &output, const NodePtr &args1,
           const NodePtr &args2) {
    OpenFheCiphertext &input1 =
        std::get<OpenFheCiphertext>(m_objects.at(args1));
    std::visit(Overloaded{[&](const OpenFheCiphertext &input2) {
                            output = context->EvalSub(input1, input2);
                          },
                          [&](const OpenFhePlaintext &input2) {
                            output = context->EvalSub(input1, input2);
                          },
                          [&](const std::vector<T> &input2) {
                            warn("Unsupported operation encountered");
                            return;
                          }},
               m_objects.at(args2));
  }

  /**
   * @brief plain_sub
   * @param [out] out The result of the subtraction operation is of type OpenFheCiphertext
   * @param [in] args1 The left operand node is a plaintext node
   * @param [in] args2 Right operand node
   */
  void plain_sub(OpenFheCiphertext &output, const NodePtr &args1,
                 const NodePtr &args2) {
    OpenFhePlaintext &input1 = std::get<OpenFhePlaintext>(m_objects.at(args1));

    std::visit(Overloaded{[&](const OpenFheCiphertext &input2) {
                            output = context->EvalSub(input1, input2);
                          },
                          [&](const OpenFhePlaintext &input2) {
                            warn("Unsupported operation encountered");
                            return;
                          },
                          [&](const std::vector<T> &input2) {
                            warn("Unsupported operation encountered");
                            return;
                          }},
               m_objects.at(args2));
  }

  /**
   * @brief mul
   * @param [out] out The result of the multiplication operation is of type OpenFheCiphertext
   * @param [in] args1 Left operand node
   * @param [in] args2 Right operand node
   */
  void mul(OpenFheCiphertext &output, const NodePtr &args1,
           const NodePtr &args2) {
    // swap args if arg1 is plain type and arg2 is of cipher type
    if (!isCipher(args1) && isCipher(args2)) {
      mul(output, args2, args1);
      return;
    }
    OpenFheCiphertext &input1 =
        std::get<OpenFheCiphertext>(m_objects.at(args1));

    std::visit(
        Overloaded{
            [&](const OpenFheCiphertext &input2) {
              // if (input1->GetLevel() != input2->GetLevel()) {
              //   LOG(LOGLEVEL::Debug, "diff mult level input1 %u , input2 %u",
              //       input1->GetLevel(), input2->GetLevel());
              // }

              output = context->EvalMult(input1, input2);
              // 做完mul  level+1

              if (dag.m_enable_bootstrap) {
                uint32_t cur_level = output->GetLevel();
                /*
                Reaching the maximum achievable multiplication depth for the first time
                 LEVELS_BEFORE_BOOTSTRAP ： Scenario two: Bootstraping has been performed
                 Up to the maximum multiplication depth - 2, 
               */
                if ((cur_level >= MAX_MULT_DEPTH_NO_BOOT &&
                     cur_level < (m_final_depth - LEVELS_BEFORE_BOOTSTRAP)) ||
                    (cur_level >= (m_final_depth - 2))) {
                  LOG(LOGLEVEL::Debug, "need bootstrap ciper_level %u",
                      output->GetLevel());
                  timespec start_time = gettime();
                  output = context->EvalBootstrap(output);
                  timespec end_time = gettime();
                  LOG(LOGLEVEL::Debug,
                      "after bootstrap  ciper_level %u, timecost %f ms",
                      output->GetLevel(), time_diff(start_time, end_time));
                }
              }
            },
            [&](const OpenFhePlaintext &input2) {
              output = context->EvalMult(input1, input2);
            },
            [&](const std::vector<T> &input2) {
              warn("Unsupported operation encountered");
              return;
            }},
        m_objects.at(args2));
  }

  /**
   * @brief leftRotate ciphertext
   */
  void leftRotate(OpenFheCiphertext &output, const NodePtr &args1,
                  std::int32_t rotation) {
    OpenFheCiphertext &input1 =
        std::get<OpenFheCiphertext>(m_objects.at(args1));
    output = context->EvalRotate(input1, rotation);
  }

  /**
   * @brief rightRotate ciphertext
   */
  void rightRotate(OpenFheCiphertext &output, const NodePtr &args1,
                   std::int32_t rotation) {
    OpenFheCiphertext &input1 =
        std::get<OpenFheCiphertext>(m_objects.at(args1));
    output = context->EvalRotate(input1, -rotation);
  }

  /**
   * @brief negate ciphertext
   */
  void negate(OpenFheCiphertext &output, const NodePtr &args1) {
    OpenFheCiphertext &input1 =
        std::get<OpenFheCiphertext>(m_objects.at(args1));
    output = context->EvalNegate(input1);
  }

  /**
   * @brief relinearize ciphertext
   */
  void relinearize(OpenFheCiphertext &output, const NodePtr &args1) {
    OpenFheCiphertext &input1 =
        std::get<OpenFheCiphertext>(m_objects.at(args1));
    output = context->Relinearize(input1);
  }

  /**
   * @brief rescale ciphertext
   */
  void rescale(OpenFheCiphertext &output, const NodePtr &args1,
               std::uint32_t divisor) {
    OpenFheCiphertext &input1 =
        std::get<OpenFheCiphertext>(m_objects.at(args1));
    output = context->Rescale(input1);
  }

  void expandConstant(std::vector<T> &output,
                      const std::shared_ptr<ConstantValue<T>> constant) {
    constant->expandTo(output, dag.getVecSize());
  }

  template <typename T_VALUE>
  T_VALUE &initValue(const NodePtr &node) {
    return std::get<T_VALUE>(m_objects[node] = T_VALUE{});
  }

 public:
  /**
   * @brief OpenfheExecutor constructor
   * @param [in] g execution graph
   * @param [in] ctx openfhe的context
   * @param [in] final_depth Final multiplication depth after graph optimization
   */
  OpenfheExecutor(Dag &g, OpenFheContext ctx, uint32_t final_depth)
      : dag(g), context(ctx), m_objects(g), m_final_depth(final_depth) {}

  bool IsErr() { return m_has_err; }

  /**
   * @brief encodeRaw need to be encoded first
   */
  virtual int encodeRaw(OpenFhePlaintext &output, const NodePtr &args1,
                        uint32_t scale, uint32_t level) {
    warn("err in base encoderaw");
    return 0;
  }

  /**
   * @brief setInputs Setting input before execution
   */
  virtual void setInputs(const OpenFheValuation &inputs) {
    warn("err in base setInputs");
    return;
  }

  /**
   * @brief handleConstantNode Processing constant data type nodes
   */
  virtual void handleConstantNode(const NodePtr &node) {
    warn("err in base handleConstantNode");
    return;
  }

  /**
   * @brief Overload () to traverse and execute nodes in the graph
   */
  void operator()(const NodePtr &node) {
    if (logLevelLeast(LOGLEVEL::Trace)) {
      printf("IYFC : Execute t%lu = %s(", node->m_index,
             getOpName(node->m_op_type).c_str());

      bool first = true;
      for (auto &operand : node->getOperands()) {
        if (first) {
          first = false;
          printf("t%lu", operand->m_index);
        } else {
          printf(",t%lu", operand->m_index);
        }
      }
      printf(")\n");
    }

    if (m_has_err) {
      throw std::logic_error("exe err");
    }

    if (node->m_op_type == OpType::Input) return;
    auto args = node->getOperands();
    switch (node->m_op_type) {
      case OpType::Constant: {
        handleConstantNode(node);
      } break;
      case OpType::Encode: {
        OPENFHE_EXE_CHECK_ERROR(args.size() == 1,
                                "exe dag err:encode arg size != 1");
        OPENFHE_EXE_CHECK_ERROR(isRaw(args[0]),
                                "exe dag err:encode arg not raw type");
        auto &output = initValue<OpenFhePlaintext>(node);
        if (0 != this->encodeRaw(output, args[0], 0, 0)) {
          m_has_err = true;
          return;
        }
      } break;
      case OpType::Add:
        OPENFHE_EXE_CHECK_ERROR(args.size() == 2, "exe dag err:add args !=2");
        if (isRaw(args[0]) && isRaw(args[1])) {
          auto &output = initValue<std::vector<T>>(node);
          binOpRaw<std::plus<T>>(output, args[0], args[1]);
        } else {  // handles plain and cipher
          OPENFHE_EXE_CHECK_ERROR(isCipher(args[0]) || isPlain(args[0]),
                                  "add arg0 type err");
          OPENFHE_EXE_CHECK_ERROR(isCipher(args[1]) || isPlain(args[1]),
                                  "add arg1 type err");
          auto &output = initValue<OpenFheCiphertext>(node);
          add(output, args[0], args[1]);
        }
        break;
      case OpType::Sub:

        OPENFHE_EXE_CHECK_ERROR(args.size() == 2, "exe dag err:sub args !=2");
        if (isRaw(args[0]) && isRaw(args[1])) {
          auto &output = initValue<std::vector<T>>(node);
          binOpRaw<std::minus<T>>(output, args[0], args[1]);
        } else {  // handles plain and cipher
          OPENFHE_EXE_CHECK_ERROR(isCipher(args[0]) || isPlain(args[0]),
                                  "sub arg0 type err");
          OPENFHE_EXE_CHECK_ERROR(isCipher(args[1]) || isPlain(args[1]),
                                  "sub arg1 type err");

          if (isPlain(args[0])) {
            auto &output = initValue<OpenFheCiphertext>(node);
            plain_sub(output, args[0], args[1]);
          } else {
            auto &output = initValue<OpenFheCiphertext>(node);
            sub(output, args[0], args[1]);
          }
        }
        break;
      case OpType::Mul:
        OPENFHE_EXE_CHECK_ERROR(args.size() == 2, "exe dag err:mub args !=2");
        if (isRaw(args[0]) && isRaw(args[1])) {
          auto &output = initValue<std::vector<T>>(node);
          binOpRaw<std::multiplies<T>>(output, args[0], args[1]);
        } else {  // works on cipher, no plaintext support
          OPENFHE_EXE_CHECK_ERROR(isCipher(args[0]) || isCipher(args[1]),
                                  "mul :one of args must be cipher");

          OPENFHE_EXE_CHECK_ERROR(!isRaw(args[0]) && !isRaw(args[1]),
                                  "mul :works on cipher, no plaintext support");

          auto &output = initValue<OpenFheCiphertext>(node);
          mul(output, args[0], args[1]);
        }
        break;
      case OpType::RotateLeftConst:
        OPENFHE_EXE_CHECK_ERROR(args.size() == 1,
                                "exe dag err:RotateLeftConst args !=1");
        if (isRaw(args[0])) {
          auto &output = initValue<std::vector<T>>(node);
          leftRotateRaw(output, args[0], node->get<RotationAttr>());
        } else {  // works on cipher, no plaintext support
          OPENFHE_EXE_CHECK_ERROR(
              isCipher(args[0]),
              "RotateLeftConst : on cipher, no plaintext support");
          auto &output = initValue<OpenFheCiphertext>(node);
          leftRotate(output, args[0], node->get<RotationAttr>());
        }
        break;
      case OpType::RotateRightConst:
        OPENFHE_EXE_CHECK_ERROR(args.size() == 1,
                                "exe dag err:RotateRightConst args !=1");
        if (isRaw(args[0])) {
          auto &output = initValue<std::vector<T>>(node);
          rightRotateRaw(output, args[0], node->get<RotationAttr>());
        } else {  // works on cipher, no plaintext support
          OPENFHE_EXE_CHECK_ERROR(
              isCipher(args[0]),
              "RotateRightConst : on cipher, no plaintext support");
          auto &output = initValue<OpenFheCiphertext>(node);
          rightRotate(output, args[0], node->get<RotationAttr>());
        }
        break;
      case OpType::Negate:
        OPENFHE_EXE_CHECK_ERROR(args.size() == 1,
                                "exe dag err:Negate args !=1");
        if (isRaw(args[0])) {
          auto &output = initValue<std::vector<T>>(node);
          negateRaw(output, args[0]);
        } else {  // works on cipher, no plaintext support
          OPENFHE_EXE_CHECK_ERROR(isCipher(args[0]),
                                  "Negate : on cipher, no plaintext support");
          auto &output = initValue<OpenFheCiphertext>(node);
          negate(output, args[0]);
        }
        break;
      case OpType::Relinearize: {
        OPENFHE_EXE_CHECK_ERROR(args.size() == 1,
                                "exe dag err:Relinearize args !=1");
        OPENFHE_EXE_CHECK_ERROR(
            isCipher(args[0]), "Relinearize : on cipher, no plaintext support");
        auto &output = initValue<OpenFheCiphertext>(node);
        relinearize(output, args[0]);
      } break;
      case OpType::Rescale: {
        OPENFHE_EXE_CHECK_ERROR(args.size() == 1,
                                "exe dag err:Rescale args !=1");
        OPENFHE_EXE_CHECK_ERROR(isCipher(args[0]),
                                "Rescale : on cipher, no plaintext support");
        auto &output = initValue<OpenFheCiphertext>(node);
        rescale(output, args[0], node->get<RescaleDivisorAttr>());
      } break;
      case OpType::Output: {
        OPENFHE_EXE_CHECK_ERROR(args.size() == 1,
                                "exe dag err:Output args !=1");
        m_objects[node] = m_objects.at(args[0]);
      } break;
      default:
        warn("Unhandled m_op_type %s", getOpName(node->m_op_type).c_str());
        m_has_err = true;
        return;
    }
  }

  void getOutputs(OpenFheValuation &enc_outputs) {
    for (auto &out : dag.getOutputs()) {
      std::visit(Overloaded{[&](const OpenFheCiphertext &output) {
                              enc_outputs[out.first] = output;
                            },
                            [&](const OpenFhePlaintext &output) {
                              enc_outputs[out.first] = output;
                            },
                            [&](const std::vector<T> &output) {
                              enc_outputs[out.first] =
                                  std::make_shared<DenseConstantValue<T>>(
                                      dag.getVecSize(), output);
                            }},
                 m_objects.at(out.second));
    }
  }
};

/**
 * @class CkksOpenFheExecutor :Specific CKKS algorithm execution class inherits from OpenfheExecutor
 * @brief Function needs to be rewritten:  encodeRaw setInputs handleConstantNode
 */

class CkksOpenFheExecutor : public OpenfheExecutor<double> {
 public:
  CkksOpenFheExecutor(Dag &g, OpenFheContext ctx, uint32_t final_depth)
      : OpenfheExecutor<double>(g, ctx, final_depth) {}

  int encodeRaw(OpenFhePlaintext &output, const NodePtr &args1, uint32_t scale,
                uint32_t level) {
    auto &in = std::get<std::vector<double>>(m_objects.at(args1));
    output = context->MakeCKKSPackedPlaintext(in);
    return 0;
  }

  virtual void setInputs(const OpenFheValuation &inputs) {
    for (auto &in : inputs) {
      auto node = dag.getInput(in.first);
      std::visit(
          Overloaded{
              [&](const OpenFheCiphertext &input) { m_objects[node] = input; },
              [&](const OpenFhePlaintext &input) { m_objects[node] = input; },
              [&](const std::shared_ptr<ConstantValue<double>> &input) {
                auto &value = initValue<std::vector<double>>(node);
                this->expandConstant(value, input);
              },
              [&](const std::shared_ptr<ConstantValue<int64_t>> &input) {
                warn("err input type %s");
                return;
              }},
          in.second);
    }
  }

  virtual void handleConstantNode(const NodePtr &node) {
    auto &output = initValue<std::vector<double>>(node);
    std::shared_ptr<ConstantValue<double>> ptr1 = node->get<ConstValueAttr>();
    this->expandConstant(output, ptr1);
    return;
  }
};

/**
 * @class BfvOpenfheExecutor :Specific BFV algorithm execution class inherits from OpenfheExecutor
 * @brief Function needs to be rewritten: encodeRaw setInputs handleConstantNode
 */
class BfvOpenfheExecutor : public OpenfheExecutor<int64_t> {
 public:
  BfvOpenfheExecutor(Dag &g, OpenFheContext ctx, uint32_t final_depth)
      : OpenfheExecutor<int64_t>(g, ctx, final_depth) {}

  int encodeRaw(OpenFhePlaintext &output, const NodePtr &args1, uint32_t scale,
                uint32_t level) {
    auto &in = std::get<std::vector<int64_t>>(m_objects.at(args1));
   
    output = context->MakePackedPlaintext(in);
    return 0;
  }

  virtual void setInputs(const OpenFheValuation &inputs) {
    for (auto &in : inputs) {
      auto node = dag.getInput(in.first);
      std::visit(
          Overloaded{
              [&](const OpenFheCiphertext &input) { m_objects[node] = input; },
              [&](const OpenFhePlaintext &input) { m_objects[node] = input; },
              [&](const std::shared_ptr<ConstantValue<int64_t>> &input) {
                auto &value = initValue<std::vector<int64_t>>(node);
                this->expandConstant(value, input);
              },
              [&](const std::shared_ptr<ConstantValue<double>> &input) {
                warn("err input type double");
                return;
              }},
          in.second);
    }
  }

  virtual void handleConstantNode(const NodePtr &node) {
    auto &output = initValue<std::vector<int64_t>>(node);
    std::shared_ptr<ConstantValue<int64_t>> ptr1 =
        node->get<ConstValueInt64Attr>();
    this->expandConstant(output, ptr1);
    return;
  }
};

}  // namespace iyfc