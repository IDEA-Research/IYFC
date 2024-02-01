/*
 * ConcreteExecutor :Concrete library's core execution logic class
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
#include <variant>
#include <vector>

#include "comm_include.h"
#include "concrete_value.h"
#include "dag/iyfc_dag.h"
#include "dag/node_map.h"
#include "libforc/concrete_header.h"
#include "util/logging.h"
#include "util/overloaded.h"

namespace iyfc {

/**
 * @class ConcreteExecutor
 * @brief Class for executing the concrete library.
 */

class ConcreteExecutor {
  // concrete runtime value
  using ConcreteRuntimeValue =
      std::variant<std::shared_ptr<ConcreteCipher>, uint32_t>;

  Dag &dag;
  ServerKey *m_server_key;

  NodeMapOptional<ConcreteRuntimeValue> m_objects;
  // check is cipher node
  bool isCipher(const NodePtr &t) {
    return std::holds_alternative<std::shared_ptr<ConcreteCipher>>(
        m_objects.at(t));
  }
  // check is plain node
  bool isPlain(const NodePtr &t) {
    return std::holds_alternative<uint32_t>(m_objects.at(t));
  }

  void leftRotate(std::shared_ptr<ConcreteCipher> &output, const NodePtr &args1,
                  std::uint32_t rotation) {
    assert(isCipher(args1));
    std::shared_ptr<ConcreteCipher> &input1 =
        std::get<std::shared_ptr<ConcreteCipher>>(m_objects.at(args1));
    output->m_fhe_value =
        c_left_rotate(m_server_key, input1->m_fhe_value, rotation);
  }

  void rightRotate(std::shared_ptr<ConcreteCipher> &output,
                   const NodePtr &args1, uint32_t rotation) {
    assert(isCipher(args1));
    std::shared_ptr<ConcreteCipher> &input1 =
        std::get<std::shared_ptr<ConcreteCipher>>(m_objects.at(args1));
    output->m_fhe_value =
        c_right_rotate(m_server_key, input1->m_fhe_value, rotation);
  }

  void negate(std::shared_ptr<ConcreteCipher> &output, const NodePtr &args1) {
    assert(isCipher(args1));
    std::shared_ptr<ConcreteCipher> &input1 =
        std::get<std::shared_ptr<ConcreteCipher>>(m_objects.at(args1));
    output->m_fhe_value = c_negate(m_server_key, input1->m_fhe_value);
  }

  void add(std::shared_ptr<ConcreteCipher> &output, const NodePtr &args1,
           const NodePtr &args2) {
    // must at least one cipher node
    if (!isCipher(args1)) {
      assert(isCipher(args2));
      add(output, args2, args1);
      return;
    }
    std::shared_ptr<ConcreteCipher> &input1 =
        std::get<std::shared_ptr<ConcreteCipher>>(m_objects.at(args1));
    // TODO: should a previous lowering get rid of this dispatch?
    std::visit(Overloaded{[&](const std::shared_ptr<ConcreteCipher> &input2) {
                            // cipher add cipher
                            output->m_fhe_value = c_executor_add(
                                m_server_key, input1->m_fhe_value,
                                input2->m_fhe_value);
                          },
                          [&](const uint32_t &input2) {
                            // cipher add plain
                            output->m_fhe_value = c_executor_add_plain(
                                m_server_key, input1->m_fhe_value, input2);
                          }},
               m_objects.at(args2));
  }

  void sub(std::shared_ptr<ConcreteCipher> &output, const NodePtr &args1,
           const NodePtr &args2) {
    if (isCipher(args2)) {
      std::shared_ptr<ConcreteCipher> &input1 =
          std::get<std::shared_ptr<ConcreteCipher>>(m_objects.at(args1));
      std::visit(Overloaded{[&](const std::shared_ptr<ConcreteCipher> &input2) {
                              output->m_fhe_value = c_executor_sub(
                                  m_server_key, input1->m_fhe_value,
                                  input2->m_fhe_value);
                            },
                            [&](const uint32_t &input2) {
                              output->m_fhe_value = c_executor_sub_plain(
                                  m_server_key, input1->m_fhe_value, input2);
                            }},
                 m_objects.at(args2));
    } else if (isCipher(args2)) {
      uint32_t &input1 = std::get<uint32_t>(m_objects.at(args1));
      std::shared_ptr<ConcreteCipher> &input2 =
          std::get<std::shared_ptr<ConcreteCipher>>(m_objects.at(args2));
      output->m_fhe_value =
          c_executor_plain_sub(m_server_key, input1, input2->m_fhe_value);
    }
  }

  void mul(std::shared_ptr<ConcreteCipher> &output, const NodePtr &args1,
           const NodePtr &args2) {
    // swap args if arg1 is plain type and arg2 is of cipher type
    if (!isCipher(args1) && isCipher(args2)) {
      mul(output, args2, args1);
      return;
    }
    std::shared_ptr<ConcreteCipher> &input1 =
        std::get<std::shared_ptr<ConcreteCipher>>(m_objects.at(args1));
    std::visit(Overloaded{[&](const std::shared_ptr<ConcreteCipher> &input2) {
                            output->m_fhe_value = c_executor_mul(
                                m_server_key, input1->m_fhe_value,
                                input2->m_fhe_value);
                          },
                          [&](const uint32_t &input2) {
                            output->m_fhe_value = c_executor_mul_plain(
                                m_server_key, input1->m_fhe_value, input2);
                          }},
               m_objects.at(args2));
  }

  // div in concrete
  void div(std::shared_ptr<ConcreteCipher> &output, const NodePtr &args1,
           const NodePtr &args2) {
    if (isCipher(args1)) {
      std::shared_ptr<ConcreteCipher> &input1 =
          std::get<std::shared_ptr<ConcreteCipher>>(m_objects.at(args1));

      std::visit(
          Overloaded{
              [&](const std::shared_ptr<ConcreteCipher> &input2) {
                output->m_fhe_value = c_executor_div(
                    m_server_key, input1->m_fhe_value, input2->m_fhe_value);
              },
              [&](const uint32_t &input2) {
                if (input2 == 0) {
                  throw std::invalid_argument("div 0");
                }
                if (!input1->m_fhe_value) {
                  throw std::invalid_argument("div input1->m_fhe_value null");
                }
                if (!m_server_key) {
                  throw std::invalid_argument("div m_server_key null");
                }
                output->m_fhe_value = c_executor_div_plain(
                    m_server_key, input1->m_fhe_value, input2);
              }},
          m_objects.at(args2));
    } else if (isCipher(args2)) {
      uint32_t &input1 = std::get<uint32_t>(m_objects.at(args1));
      std::shared_ptr<ConcreteCipher> &input2 =
          std::get<std::shared_ptr<ConcreteCipher>>(m_objects.at(args2));
      output->m_fhe_value =
          c_executor_plain_div(m_server_key, input1, input2->m_fhe_value);
    }
  }

  template <typename T>
  T &initValue(const NodePtr &node) {
    return std::get<T>(m_objects[node] = T{});
  }

  // init ptr value
  template <typename T>
  std::shared_ptr<T> &initSharedPtrValue(const NodePtr &node) {
    return std::get<std::shared_ptr<T>>(m_objects[node] =
                                            std::make_shared<T>());
  }

 public:
  ConcreteExecutor(Dag &g, ServerKey *server_key)
      : dag(g), m_server_key(server_key), m_objects(g) {}

  /**
   * @brief      setInputs: setInputs before exe a  dag
   * @details
   * @param[in]  inputs   const ConcreteValuation
   */
  void setInputs(const ConcreteValuation &inputs) {
    for (auto &in : inputs) {
      auto node = dag.getInput(in.first);
      std::visit(
          Overloaded{[&](const std::shared_ptr<ConcreteCipher> &input) {
                       m_objects[node] = input;
                     },
                     [&](const uint32_t &input) { m_objects[node] = input; }},
          in.second);
    }
  }
  /**
   * @brief      To traverse and execute DAG nodes.
   * @param[in]  node The specific node in the DAG.
   */
  void operator()(const NodePtr &node) {
    // log info
    if (logLevelLeast(LOGLEVEL::Debug)) {
      printf("iyfc: Execute t%lu = %s(", node->m_index,
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
      fflush(stdout);
    }

    if (node->m_op_type == OpType::Input) {
      auto input_args = node->getOperands();
      return;
    }
    auto args = node->getOperands();
    switch (node->m_op_type) {
      case OpType::U32Constant: {
        auto &output = initValue<uint32_t>(node);
        output = node->get<Uint32ConstAttr>();
      } break;
      case OpType::Add: {
        assert(args.size() == 2);
        assert(isCipher(args[0]) || isPlain(args[0]));
        assert(isCipher(args[1]) || isPlain(args[1]));
        auto &output = initSharedPtrValue<ConcreteCipher>(node);
        add(output, args[0], args[1]);
      } break;
      case OpType::Sub: {
        assert(args.size() == 2);
        assert(isCipher(args[0]) || isPlain(args[0]));
        assert(isCipher(args[1]) || isPlain(args[1]));
        auto &output = initSharedPtrValue<ConcreteCipher>(node);
        sub(output, args[0], args[1]);
      } break;
      case OpType::Mul: {
        assert(args.size() == 2);
        assert(isCipher(args[0]) || isCipher(args[1]));
        auto &output = initSharedPtrValue<ConcreteCipher>(node);
        mul(output, args[0], args[1]);
      } break;
      case OpType::Div: {
        assert(args.size() == 2);
        assert(isCipher(args[0]) || isPlain(args[0]));
        assert(isCipher(args[1]) || isPlain(args[1]));
        auto &output = initSharedPtrValue<ConcreteCipher>(node);
        div(output, args[0], args[1]);
      } break;
      case OpType::RotateLeftConst: {
        assert(args.size() == 1);
        assert(isCipher(args[0]));
        auto &output = initSharedPtrValue<ConcreteCipher>(node);
        leftRotate(output, args[0], node->get<RotationAttr>());
      } break;
      case OpType::RotateRightConst: {
        assert(args.size() == 1);
        assert(isCipher(args[0]));
        auto &output = initSharedPtrValue<ConcreteCipher>(node);
        rightRotate(output, args[0], node->get<RotationAttr>());
      } break;
      case OpType::Negate: {
        assert(args.size() == 1);
        assert(isCipher(args[0]));
        auto &output = initSharedPtrValue<ConcreteCipher>(node);
        negate(output, args[0]);
      } break;
      case OpType::Output: {
        assert(args.size() == 1);
        m_objects[node] = m_objects.at(args[0]);
      } break;
      default:
        warn("Unhandled op_type %s", getOpName(node->m_op_type).c_str());
        return;
    }
  }

  void free(const NodePtr &node) {
    if (node->m_op_type == OpType::Output) {
      return;
    }

    auto &obj = m_objects.at(node);
    std::visit(Overloaded{[](std::shared_ptr<ConcreteCipher> &cipher) {
                            cipher->release();
                          },
                          [](uint32_t &plain) { plain = 0; }},
               obj);
  }
  /**
   * @brief       Get the final encrypted results after execution.
   * @param[out]  enc_outputs  The map to store the encrypted results.
   */
  void getOutputs(ConcreteValuation &enc_outputs) {
    for (auto &out : dag.getOutputs()) {
      std::visit(
          Overloaded{
              [&](const std::shared_ptr<ConcreteCipher> &output) {
                if (!output->m_fhe_value) {
                  throw std::invalid_argument("getOutputs fhe value null");
                }
                enc_outputs[out.first] = output;
              },
              [&](const uint32_t output) { enc_outputs[out.first] = output; }},
          m_objects.at(out.second));
    }
    return;
  }
};

}  // namespace iyfc
