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
#include <seal/seal.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <variant>
#include <vector>

#include "comm_include.h"
#include "dag/constant_value.h"
#include "dag/iyfc_dag.h"
#include "dag/node_map.h"
#include "daghandler/node_degree_cnt.h"
#include "daghandler/traversal_handler.h"
#include "err_code.h"
#include "seal_encoder.h"
#include "seal_valuation.h"
#include "util/logging.h"
#include "util/overloaded.h"
#include "util/timer.h"

namespace iyfc {

#ifndef SEAL_EXE_CHECK_ERROR
#define SEAL_EXE_CHECK_ERROR(func_ret, err_msg) \
  if (!(func_ret)) {                            \
    warn("dag exe err: %s", err_msg);           \
    m_has_err = true;                           \
    return;                                     \
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
 * @class SEALExecutor
 * @brief seal library execution class traverses the nodes in the execution DAG
 */
template <typename T>
class SEALExecutor {
 public:
  using RuntimeValue = std::variant<seal::Ciphertext, seal::Plaintext,
                                    std::vector<T>>;  // seal runtime

  Dag &dag;
  seal::SEALContext context;  // context
  seal::Encryptor &encryptor;

#ifdef SEAL_4_0
  const seal::Evaluator &evaluator;
#else
  seal::Evaluator &evaluator;
#endif

  seal::GaloisKeys &m_galois_keys;
  seal::RelinKeys &m_relin_keys;
  NodeMapOptional<RuntimeValue> m_objects;
  std::unordered_map<uint64_t, int>
      m_index2out;                               // node  out-degree information
  std::unordered_map<uint64_t, int> m_index2in;  // node  in-degree information
  bool m_has_err{false};
  std::vector<T> temp_vec;

  bool isCipher(const NodePtr &t) {
    return std::holds_alternative<seal::Ciphertext>(m_objects.at(t));
  }
  bool isPlain(const NodePtr &t) {
    return std::holds_alternative<seal::Plaintext>(m_objects.at(t));
  }
  bool isRaw(const NodePtr &t) {
    return std::holds_alternative<std::vector<T>>(m_objects.at(t));
  }

  /**
   * @brief right_rotate_raw raw Node data right rotation
   */
  void right_rotate_raw(std::vector<T> &out, const NodePtr &args1,
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
   * @brief left_rotate_raw raw Node data right rotation
   */
  void left_rotate_raw(std::vector<T> &out, const NodePtr &args1,
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
   * @brief bin_op_raw raw Node data binary operations
   */
  template <class OpType>
  void bin_op_raw(std::vector<T> &out, const NodePtr &args1,
                  const NodePtr &args2) {
    auto &in1 = std::get<std::vector<T>>(m_objects.at(args1));
    auto &in2 = std::get<std::vector<T>>(m_objects.at(args2));

    SEAL_EXE_CHECK_ERROR(in1.size() == in2.size(),
                         "binopraw arg1.size 1= arg2.size");

    out.clear();
    out.reserve(in1.size());
    transform(in1.cbegin(), in1.cend(), in2.cbegin(), back_inserter(out),
              OpType());
  }

  /**
   * @brief bin_op_raw raw Node data gets negate
   */
  void negate_raw(std::vector<T> &out, const NodePtr &args1) {
    auto &in = std::get<std::vector<T>>(m_objects.at(args1));

    out.clear();
    out.reserve(in.size());
    transform(in.cbegin(), in.cend(), back_inserter(out), std::negate<T>());
  }

  /**
   * @brief add Ciphertext add operation
   */
  void add(seal::Ciphertext &output, const NodePtr &args1,
           const NodePtr &args2) {
    if (!isCipher(args1)) {
      SEAL_EXE_CHECK_ERROR(isCipher(args2),
                           " add: arg1 or arg2 must be cipher ");

      add(output, args2, args1);
      return;
    }
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    // input2 Different types of operations
    std::visit(Overloaded{[&](const seal::Ciphertext &input2) {
                            evaluator.add(input1, input2, output);
                          },
                          [&](const seal::Plaintext &input2) {
                            evaluator.add_plain(input1, input2, output);
                          },
                          [&](const std::vector<T> &input2) {
                            warn("Unsupported operation encountered");
                            return;
                          }},
               m_objects.at(args2));
  }

  /**
   * @brief add_inplace Ciphertext add_inplace operation can reduce memory
   * copying
   */
  void add_inplace(NodePtr &args1, NodePtr &args2, const NodePtr &node) {
    if (!isCipher(args1)) {
      add_inplace(args2, args1, node);
      return;
    }
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    std::visit(Overloaded{[&](const seal::Ciphertext &input2) {
                            evaluator.add_inplace(input1, input2);
                          },
                          [&](const seal::Plaintext &input2) {
                            evaluator.add_plain_inplace(input1, input2);
                          },
                          [&](const std::vector<T> &input2) {
                            warn("Unsupported operation encountered");
                            return;
                          }},
               m_objects.at(args2));
    m_objects[node] = std::move(m_objects.at(args1));
  }

  /**
   * @brief sub Ciphertext sub operation
   */
  void sub(seal::Ciphertext &output, const NodePtr &args1,
           const NodePtr &args2) {
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    std::visit(Overloaded{[&](const seal::Ciphertext &input2) {
                            evaluator.sub(input1, input2, output);
                          },
                          [&](const seal::Plaintext &input2) {
                            evaluator.sub_plain(input1, input2, output);
                          },
                          [&](const std::vector<T> &input2) {
                            warn("Unsupported operation encountered");
                            return;
                          }},
               m_objects.at(args2));
  }

  /**
   * @brief sub_inplace Ciphertext sub_inplace operation reduces memory copying
   */
  void sub_inplace(NodePtr &args1, const NodePtr &args2, const NodePtr &node) {
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    std::visit(Overloaded{[&](const seal::Ciphertext &input2) {
                            evaluator.sub_inplace(input1, input2);
                          },
                          [&](const seal::Plaintext &input2) {
                            evaluator.sub_plain_inplace(input1, input2);
                          },
                          [&](const std::vector<T> &input2) {
                            warn("Unsupported operation encountered");
                            return;
                          }},
               m_objects.at(args2));
    m_objects[node] = std::move(m_objects.at(args1));
  }

  /**
   * @brief plain_sub Plain text - cipher text
   */
  void plain_sub(seal::Ciphertext &output, const NodePtr &args1,
                 const NodePtr &args2) {
    seal::Plaintext &input1 = std::get<seal::Plaintext>(m_objects.at(args1));

    std::visit(Overloaded{[&](const seal::Ciphertext &input2) {
                            // 转成 -ciper + plain
                            seal::Ciphertext tmp_negate;
                            evaluator.negate(input2, tmp_negate);
                            evaluator.add_plain(tmp_negate, input1, output);
                          },
                          [&](const seal::Plaintext &input2) {
                            warn("Unsupported operation encountered");
                          },
                          [&](const std::vector<T> &input2) {
                            warn("Unsupported operation encountered");
                            return;
                          }},
               m_objects.at(args2));
  }

  /**
   * @brief mul plaintext multiplication
   */
  void mul(seal::Ciphertext &output, const NodePtr &args1,
           const NodePtr &args2) {
    // swap args if arg1 is plain type and arg2 is of cipher type
    if (!isCipher(args1) && isCipher(args2)) {
      mul(output, args2, args1);
      return;
    }
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    std::visit(Overloaded{[&](const seal::Ciphertext &input2) {
                            if (args1 == args2) {
                              evaluator.square(input1, output);
                            } else {
                              evaluator.multiply(input1, input2, output);
                            }
                          },
                          [&](const seal::Plaintext &input2) {
                            evaluator.multiply_plain(input1, input2, output);
                          },
                          [&](const std::vector<T> &input2) {
                            warn("Unsupported operation encountered");
                            return;
                          }},
               m_objects.at(args2));
  }

  /**
   * @brief mul_inplace Plaintext multiplication   inplace reduces memory
   * copying
   */
  void mul_inplace(NodePtr &args1, NodePtr &args2, const NodePtr &node) {
    // swap args if arg1 is plain type and arg2 is of cipher type
    if (!isCipher(args1) && isCipher(args2)) {
      mul_inplace(args2, args1, node);
      return;
    }
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    std::visit(Overloaded{[&](const seal::Ciphertext &input2) {
                            if (args1 == args2) {
                              evaluator.square_inplace(input1);
                            } else {
                              evaluator.multiply_inplace(input1, input2);
                            }
                          },
                          [&](const seal::Plaintext &input2) {
                            evaluator.multiply_plain_inplace(input1, input2);
                          },
                          [&](const std::vector<T> &input2) {
                            warn("Unsupported operation encountered");
                            return;
                          }},
               m_objects.at(args2));
    // move reduces copying
    m_objects[node] = std::move(m_objects.at(args1));
  }

  /**
   * @brief Cipher text left-handed
   */
  void left_rotate(seal::Ciphertext &output, const NodePtr &args1,
                   std::int32_t rotation) {
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    evaluator.rotate_vector(input1, rotation, m_galois_keys, output);
  }

  /**
   * @brief Cipher text left-handed inplace
   */
  void left_rotate_inplace(NodePtr &args1, std::int32_t rotation,
                           const NodePtr &node) {
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    evaluator.rotate_vector_inplace(input1, rotation, m_galois_keys);
    m_objects[node] = std::move(m_objects.at(args1));
  }

  /**
   * @brief Cipher text right-hand
   */
  void right_rotate(seal::Ciphertext &output, const NodePtr &args1,
                    std::int32_t rotation) {
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    evaluator.rotate_vector(input1, -rotation, m_galois_keys, output);
  }

  /**
   * @brief Cipher text right-hand  inplace
   */
  void right_rotate_inplace(NodePtr &args1, std::int32_t rotation,
                            const NodePtr &node) {
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    evaluator.rotate_vector_inplace(input1, -rotation, m_galois_keys);
    m_objects[node] = std::move(m_objects.at(args1));
  }

  /**
   * @brief ciphertext get negate
   */
  void negate(seal::Ciphertext &output, const NodePtr &args1) {
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    evaluator.negate(input1, output);
  }

  /**
   * @brief negate_inplace
   */
  void negate_inplace(NodePtr &args1, const NodePtr &node) {
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    evaluator.negate_inplace(input1);
    m_objects[node] = std::move(m_objects.at(args1));
  }

  /**
   * @brief relinearize
   */
  void relinearize(seal::Ciphertext &output, const NodePtr &args1) {
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    evaluator.relinearize(input1, m_relin_keys, output);
  }

  /**
   * @brief relinearize_inplace
   */
  void relinearize_inplace(NodePtr &args1, const NodePtr &node) {
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    evaluator.relinearize_inplace(input1, m_relin_keys);
    m_objects[node] = std::move(m_objects.at(args1));
  }

  /**
   * @brief mod_switch
   */
  void mod_switch(seal::Ciphertext &output, const NodePtr &args1) {
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    evaluator.mod_switch_to_next(input1, output);
  }

  /**
   * @brief mod_switch_inplace
   */
  void mod_switch_inplace(NodePtr &args1, const NodePtr &node) {
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    evaluator.mod_switch_to_next_inplace(input1);
    m_objects[node] = std::move(m_objects.at(args1));
  }

  /**
   * @brief rescale
   */
  void rescale(seal::Ciphertext &output, const NodePtr &args1,
               std::uint32_t divisor) {
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    evaluator.rescale_to_next(input1, output);
    output.scale() = input1.scale() / pow(2.0, divisor);
  }

  /**
   * @brief rescale_inplace
   */
  void rescale_inplace(NodePtr &args1, std::uint32_t divisor,
                       const NodePtr &node) {
    seal::Ciphertext &input1 = std::get<seal::Ciphertext>(m_objects.at(args1));
    double scale = input1.scale() / pow(2.0, divisor);
    evaluator.rescale_to_next_inplace(input1);
    input1.scale() = scale;
    m_objects[node] = std::move(m_objects.at(args1));
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
  SEALExecutor(Dag &g, seal::SEALContext ctx, seal::Encryptor &enc,
               seal::Evaluator &e, seal::GaloisKeys &gk, seal::RelinKeys &rk)
      : dag(g),
        context(ctx),
        encryptor(enc),
        evaluator(e),
        m_galois_keys(gk),
        m_relin_keys(rk),
        m_objects(g) {
    m_index2out.clear();
    m_index2in.clear();
    // Get in-degree and out-degree information
    DagTraversal dag_traverse(g);
    NodeDegreeCnt degree(g, m_index2out, m_index2in);
    dag_traverse.forwardPass(degree);
  }

  bool IsErr() { return m_has_err; }
  /**
   * @brief encode_raw Encode primitive data types
   */
  virtual int encode_raw(seal::Plaintext &output, const NodePtr &args1,
                         uint32_t scale, uint32_t level) {
    warn("err in base encoderaw");
    return 0;
  }

  /**
   * @brief setInputs Set input before execution
   */
  virtual void setInputs(const SEALValuation &inputs) {
    warn("err in base setInputs");
    return;
  }

  /**
   * @brief setInputs Handling constant nodes
   */
  virtual void handleConstantNode(const NodePtr &node) {
    warn("err in base handleConstantNode");
    return;
  }

  /**
   * @brief Overloaded() operator traverses the execution DAG
   */
  void operator()(const NodePtr &node) {
    if (m_has_err) {
      throw std::logic_error("exe err");
    }
    
    if (logLevelLeast(LOGLEVEL::Trace)) {
      printf("iyfc: Execute t%lu = %s(", node->m_index,
             getOpName(node->m_op_type).c_str());

      bool first = true;
      for (const auto &operand : node->getOperands()) {
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

    if (node->m_op_type == OpType::Input) return;
    auto args = node->getOperands();
    size_t arg_size = args.size();
    // If op-- is used last, it is ok
    std::vector<bool> vec_agr_inplace(arg_size, false);
    for (int i = 0; i < arg_size; i++) {
      m_index2out[args[i]->m_index]--;
      // printf("op %d,  out_cnt %d \n", i, m_index2out[args[i]->m_index]);
      if (m_index2out[args[i]->m_index] == 0) vec_agr_inplace[i] = true;
    }

    switch (node->m_op_type) {
      case OpType::Constant: {
        handleConstantNode(node);
        /*
        auto &output = initValue<std::vector<T>>(node);
        GET_CONSTANT_NODE_ATTR
        expandConstant(output, ptr1);
        */
      } break;
      case OpType::Encode: {
        SEAL_EXE_CHECK_ERROR(arg_size == 1, "exe dag err:encode arg size != 1");
        SEAL_EXE_CHECK_ERROR(isRaw(args[0]),
                             "exe dag err:encode arg not raw type");
        auto &output = initValue<seal::Plaintext>(node);
        if (0 != this->encode_raw(output, args[0],
                                  node->get<EncodeAtScaleAttr>(),
                                  node->get<EncodeAtLevelAttr>())) {
          m_has_err = true;
          return;
        }
      } break;
      case OpType::Add:
        SEAL_EXE_CHECK_ERROR(arg_size == 2, "exe dag err:add args !=2");
        if (isRaw(args[0]) && isRaw(args[1])) {
          auto &output = initValue<std::vector<T>>(node);
          bin_op_raw<std::plus<T>>(output, args[0], args[1]);
        } else {  // handles plain and cipher
          SEAL_EXE_CHECK_ERROR(isCipher(args[0]) || isPlain(args[0]),
                               "add arg0 type err");
          SEAL_EXE_CHECK_ERROR(isCipher(args[1]) || isPlain(args[1]),
                               "add arg1 type err");
          if (vec_agr_inplace[0] && isCipher(args[0])) {
            add_inplace(args[0], args[1], node);
          } else if (vec_agr_inplace[1] && isCipher(args[1])) {
            add_inplace(args[1], args[0], node);
          } else {
            auto &output = initValue<seal::Ciphertext>(node);
            add(output, args[0], args[1]);
          }
        }
        break;
      case OpType::Sub:

        SEAL_EXE_CHECK_ERROR(arg_size == 2, "exe dag err:sub args !=2");
        if (isRaw(args[0]) && isRaw(args[1])) {
          auto &output = initValue<std::vector<T>>(node);
          bin_op_raw<std::minus<T>>(output, args[0], args[1]);
        } else {  // handles plain and cipher
          SEAL_EXE_CHECK_ERROR(isCipher(args[0]) || isPlain(args[0]),
                               "sub arg0 type err");
          SEAL_EXE_CHECK_ERROR(isCipher(args[1]) || isPlain(args[1]),
                               "sub arg1 type err");
          if (isPlain(args[0])) {
            auto &output = initValue<seal::Ciphertext>(node);
            plain_sub(output, args[0], args[1]);
          } else {
            if (vec_agr_inplace[0]) {
              sub_inplace(args[0], args[1], node);
            } else {
              auto &output = initValue<seal::Ciphertext>(node);
              sub(output, args[0], args[1]);
            }
          }
        }
        break;
      case OpType::Mul:
        SEAL_EXE_CHECK_ERROR(arg_size == 2, "exe dag err:mub args !=2");
        if (isRaw(args[0]) && isRaw(args[1])) {
          auto &output = initValue<std::vector<T>>(node);
          bin_op_raw<std::multiplies<T>>(output, args[0], args[1]);
        } else {  // works on cipher, no plaintext support
          SEAL_EXE_CHECK_ERROR(isCipher(args[0]) || isCipher(args[1]),
                               "mul :one of args must be cipher");

          SEAL_EXE_CHECK_ERROR(!isRaw(args[0]) && !isRaw(args[1]),
                               "mul :works on cipher, no plaintext support");

          if (vec_agr_inplace[0] && isCipher(args[0])) {
            mul_inplace(args[0], args[1], node);
          } else if (vec_agr_inplace[1] && isCipher(args[1])) {
            mul_inplace(args[1], args[0], node);
          } else {
            auto &output = initValue<seal::Ciphertext>(node);
            mul(output, args[0], args[1]);
          }
        }
        break;
      case OpType::RotateLeftConst:
        SEAL_EXE_CHECK_ERROR(arg_size == 1,
                             "exe dag err:RotateLeftConst args !=1");
        if (isRaw(args[0])) {
          auto &output = initValue<std::vector<T>>(node);
          left_rotate_raw(output, args[0], node->get<RotationAttr>());
        } else {  // works on cipher, no plaintext support
          SEAL_EXE_CHECK_ERROR(
              isCipher(args[0]),
              "RotateLeftConst : on cipher, no plaintext support");
          if (vec_agr_inplace[0]) {
            left_rotate_inplace(args[0], node->get<RotationAttr>(), node);
          } else {
            auto &output = initValue<seal::Ciphertext>(node);
            left_rotate(output, args[0], node->get<RotationAttr>());
          }
        }
        break;
      case OpType::RotateRightConst:
        SEAL_EXE_CHECK_ERROR(arg_size == 1,
                             "exe dag err:RotateRightConst args !=1");
        if (isRaw(args[0])) {
          auto &output = initValue<std::vector<T>>(node);
          right_rotate_raw(output, args[0], node->get<RotationAttr>());
        } else {  // works on cipher, no plaintext support
          SEAL_EXE_CHECK_ERROR(
              isCipher(args[0]),
              "RotateRightConst : on cipher, no plaintext support");
          if (vec_agr_inplace[0]) {
            right_rotate_inplace(args[0], node->get<RotationAttr>(), node);
          } else {
            auto &output = initValue<seal::Ciphertext>(node);
            right_rotate(output, args[0], node->get<RotationAttr>());
          }
        }
        break;
      case OpType::Negate:
        SEAL_EXE_CHECK_ERROR(arg_size == 1, "exe dag err:Negate args !=1");
        if (isRaw(args[0])) {
          auto &output = initValue<std::vector<T>>(node);
          negate_raw(output, args[0]);
        } else {  // works on cipher, no plaintext support
          SEAL_EXE_CHECK_ERROR(isCipher(args[0]),
                               "Negate : on cipher, no plaintext support");
          if (vec_agr_inplace[0]) {
            negate_inplace(args[0], node);
          } else {
            auto &output = initValue<seal::Ciphertext>(node);
            negate(output, args[0]);
          }
        }
        break;
      case OpType::Relinearize: {
        SEAL_EXE_CHECK_ERROR(arg_size == 1, "exe dag err:Relinearize args !=1");
        SEAL_EXE_CHECK_ERROR(isCipher(args[0]),
                             "Relinearize : on cipher, no plaintext support");
        if (vec_agr_inplace[0]) {
          relinearize_inplace(args[0], node);
        } else {
          auto &output = initValue<seal::Ciphertext>(node);
          relinearize(output, args[0]);
        }
      } break;
      case OpType::ModSwitch: {
        SEAL_EXE_CHECK_ERROR(arg_size == 1, "exe dag err:ModSwitch args !=1");
        SEAL_EXE_CHECK_ERROR(isCipher(args[0]),
                             "ModSwitch : on cipher, no plaintext support");
        if (vec_agr_inplace[0]) {
          mod_switch_inplace(args[0], node);
        } else {
          auto &output = initValue<seal::Ciphertext>(node);
          mod_switch(output, args[0]);
        }
      } break;
      case OpType::Rescale: {
        SEAL_EXE_CHECK_ERROR(arg_size == 1, "exe dag err:Rescale args !=1");
        SEAL_EXE_CHECK_ERROR(isCipher(args[0]),
                             "Rescale : on cipher, no plaintext support");
        if (vec_agr_inplace[0]) {
          rescale_inplace(args[0], node->get<RescaleDivisorAttr>(), node);
        } else {
          auto &output = initValue<seal::Ciphertext>(node);
          rescale(output, args[0], node->get<RescaleDivisorAttr>());
        }
      } break;
      case OpType::Output: {
        SEAL_EXE_CHECK_ERROR(arg_size == 1, "exe dag err:Output args !=1");
        m_objects[node] = std::move(m_objects.at(args[0]));
      } break;
      default:
        warn("Unhandled m_op_type %s", getOpName(node->m_op_type).c_str());
        m_has_err = true;
        return;
    }
  }

  void free(const NodePtr &node) {
    if (node->m_op_type == OpType::Output) {
      return;
    }
    auto &obj = m_objects.at(node);
    std::visit(Overloaded{[](seal::Ciphertext &cipher) { cipher.release(); },
                          [](seal::Plaintext &plain) { plain.release(); },
                          [](std::vector<T> &raw) {
                            raw.clear();
                            raw.shrink_to_fit();
                          }},
               obj);
  }

  /**
   * @brief getOutputs Get the ciphertext result after execution
   */
  void getOutputs(SEALValuation &enc_outputs) {
    for (auto &out : dag.getOutputs()) {
      std::visit(Overloaded{[&](const seal::Ciphertext &output) {
                              enc_outputs[out.first] = output;
                            },
                            [&](const seal::Plaintext &output) {
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
};  // namespace iyfc

/**
 * @class CkksSealExecutor inherits from SEALExecutor
 * @brief seal library ckks algorithm execution class overrides encode_raw
 * setInputs handleConstantNode function
 */
class CkksSealExecutor : public SEALExecutor<double> {
 private:
  std::shared_ptr<SealEncoderBase> encoder_ptr;

 public:
  CkksSealExecutor(std::shared_ptr<SealEncoderBase> ec, Dag &g,
                   seal::SEALContext ctx, seal::Encryptor &enc,
                   seal::Evaluator &e, seal::GaloisKeys &gk,
                   seal::RelinKeys &rk)
      : SEALExecutor<double>(g, ctx, enc, e, gk, rk), encoder_ptr(ec) {}

  virtual int encode_raw(seal::Plaintext &output, const NodePtr &args1,
                         uint32_t scale, uint32_t level) {
    auto &in = std::get<std::vector<double>>(m_objects.at(args1));

    auto ctx_data = context.first_context_data();
    for (std::size_t i = 0; i < level; ++i) {
      ctx_data = ctx_data->next_context_data();
    }

    // If the slot count is larger than the vector size, then encode repetitions
    // of the vector to fill the slot count. This will provide the correct
    // semantics for rotations.

    if (encoder_ptr->getSlotCnt() % dag.getVecSize() == 0)
      ;
    auto copies = encoder_ptr->getSlotCnt() / dag.getVecSize();
    auto &scratch = temp_vec;
    scratch.clear();
    scratch.reserve(encoder_ptr->getSlotCnt());
    for (int i = 0; i < copies; ++i) {
      scratch.insert(scratch.end(), std::begin(in), std::end(in));
    }
    try {
      encoder_ptr->setEncodePara(scale, ctx_data->parms_id());
      ValuationType src = scratch;
      encoder_ptr->encode(src, output);

      // encoder_ptr.encode(scratch, ctx_data->parms_id(), pow(2.0, scale),
      // output);

    } catch (std::invalid_argument &e) {
      warn("encode err %s", e.what());
      return SEAL_ENCODE_RAW_ERR;
    }

    return 0;
  }

  virtual void setInputs(const SEALValuation &inputs) {
    for (auto &in : inputs) {
      // LOG(LOGLEVEL::Debug, "setInputs for exe name : %s \n",
      // in.first.c_str());
      auto node = dag.getInput(in.first);

      std::visit(
          Overloaded{
              [&](const seal::Ciphertext &input) { m_objects[node] = input; },
              [&](const seal::Plaintext &input) { m_objects[node] = input; },
              [&](const std::shared_ptr<ConstantValue<double>> &input) {
                auto &value = initValue<std::vector<double>>(node);
                this->expandConstant(value, input);
              },
              [&](const std::shared_ptr<ConstantValue<int64_t>> &input) {
                warn("err input type %s");
                return;
              }},
          in.second);

      // LOG(LOGLEVEL::Debug, "setInputs for exe name : %s  success!\n",
      //     in.first.c_str());
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
 * @class BfvSealExecutor inherits from SEALExecutor
 * @brief seal library bfv algorithm execution class overrides encode_raw
 * setInputs handleConstantNode function
 */
class BfvSealExecutor : public SEALExecutor<int64_t> {
 private:
  std::shared_ptr<SealEncoderBase> encoder_ptr;

 public:
  BfvSealExecutor(std::shared_ptr<SealEncoderBase> ec, Dag &g,
                  seal::SEALContext ctx, seal::Encryptor &enc,
                  seal::Evaluator &e, seal::GaloisKeys &gk, seal::RelinKeys &rk)
      : SEALExecutor<int64_t>(g, ctx, enc, e, gk, rk), encoder_ptr(ec) {}

  int encode_raw(seal::Plaintext &output, const NodePtr &args1, uint32_t scale,
                 uint32_t level) {
    auto &in = std::get<std::vector<int64_t>>(m_objects.at(args1));

    auto ctx_data = context.first_context_data();
    for (std::size_t i = 0; i < level; ++i) {
      ctx_data = ctx_data->next_context_data();
    }

    // If the slot count is larger than the vector size, then encode repetitions
    // of the vector to fill the slot count. This will provide the correct
    // semantics for rotations.
    if (encoder_ptr->getSlotCnt() % dag.getVecSize() == 0)
      ;
    auto copies = encoder_ptr->getSlotCnt() / dag.getVecSize();

    auto &scratch = temp_vec;
    scratch.clear();
    scratch.reserve(encoder_ptr->getSlotCnt());
    for (int i = 0; i < copies; ++i) {
      scratch.insert(scratch.end(), std::begin(in), std::end(in));
    }
    try {
      encoder_ptr->setEncodePara(scale, ctx_data->parms_id());
      ValuationType src = scratch;
      encoder_ptr->encode(src, output);
      // encoder_ptr.encode(scratch, ctx_data->parms_id(), pow(2.0, scale),
      // output);

    } catch (std::invalid_argument &e) {
      throw std::logic_error("seal encode raw err %s");
    }
    return 0;
  }

  virtual void setInputs(const SEALValuation &inputs) {
    for (auto &in : inputs) {
      auto node = dag.getInput(in.first);

      std::visit(
          Overloaded{
              [&](const seal::Ciphertext &input) { m_objects[node] = input; },
              [&](const seal::Plaintext &input) { m_objects[node] = input; },
              [&](const std::shared_ptr<ConstantValue<int64_t>> &input) {
                auto &value = initValue<std::vector<int64_t>>(node);
                this->expandConstant(value, input);
              },
              [&](const std::shared_ptr<ConstantValue<double>> &input) {
                throw std::logic_error("err input type double");
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