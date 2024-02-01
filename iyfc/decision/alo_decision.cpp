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

#include "alo_decision.h"
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "daghandler/clean_node_handler.h"
#include "daghandler/mult_depth_cnt.h"
#include "daghandler/reduction_handler.h"
#include "daghandler/traversal_handler.h"
#include "daghandler/type_handler.h"
#include "err_code.h"

namespace iyfc {

AloDecision::AloDecision() { m_fhe_manager = std::make_shared<FheManager>(); }

void AloDecision::InitDagForDecision(Dag& dag) {
  LOG(LOGLEVEL::Debug, "before InitDagForDecision max_index %lu name %s",
      dag.getNextNodeIndex(), dag.getName().c_str());
  auto dag_rewrite = DagTraversal(dag);
  dag_rewrite.backwardPass(CleanNodeHandler(dag));
  dag.setScaleRange();  // Perserve
  NodeMap<DataType> types(dag);
  NodeMapOptional<std::uint32_t> scales(dag);
  // Type inference
  dag_rewrite.forwardPass(TypeHandler(dag, types));
  // Balance adjustment -- general
  dag_rewrite.forwardPass(Reduction(dag));
  dag_rewrite.forwardPass(ReductionLogExpander(dag, types));
  dag_rewrite.forwardPass(TypeHandler(dag, types));
  MultDepthCnt depth(dag, types);
  dag_rewrite.forwardPass(depth);
  dag.m_after_reduction_depth = depth.getMultDepth();
  LOG(LOGLEVEL::Debug, "after InitDagForDecision max_index %lu",
      dag.getNextNodeIndex());
}
void AloDecision::setAloName(Dag& dag, std::string& tmp_alo_name) {
  uint32_t max_dep_for_seal = MAX_SEAL_BITS / dag.m_scale - DEFAULT_Q_CNT;
  LOG(LOGLEVEL::Debug, "max_dep_for_seal %lu, sacle%lu \n", max_dep_for_seal,
      dag.m_scale);
  if (dag.supportShortInt())
    tmp_alo_name = "concrete";
  else if (dag.m_has_int64) {
    tmp_alo_name = "seal_bfv";
    if (m_max_mul_dep > max_dep_for_seal) {
      tmp_alo_name = "openfhe_bfv";
    }
  } else if (dag.m_has_double) {
    // ckks common logic processing
    tmp_alo_name = "seal_ckks";
    // Handle uint32 nodes, vector const nodes
    auto dag_rewrite = DagTraversal(dag);
    dag_rewrite.forwardPass(U32ToConstant(dag, TYPE_DOUBLE));
    // SEAL_ckks library can handle a maximum multiplication depth of 11 layers, use openfhe for more than 11 layers
    if (m_max_mul_dep > max_dep_for_seal) {
      tmp_alo_name = "openfhe_ckks";
    }

  } else {
    throw std::logic_error("err input type !");
  }
}

int AloDecision::deLibAndAlo(Dag& dag) {
  InitDagForDecision(dag);
  m_max_mul_dep = std::max(m_max_mul_dep, dag.m_after_reduction_depth);
  std::string tmp_alo_name;
  // shortint uses the concrete library
  setAloName(dag, tmp_alo_name);
  LOG(LOGLEVEL::Debug, " use alo %s, after_reduction_depth %u ",
      tmp_alo_name.c_str(), dag.m_after_reduction_depth);
  int de_ret = dePar(tmp_alo_name, dag);
  m_libs.emplace_back(std::move(tmp_alo_name));
  return 0;
}

int AloDecision::deGroupLibAndAlo(
    Dag& root_dag, std::unordered_map<std::string, DagPtr>& m_name2dag) {
  for (auto& item : m_name2dag) {
    // Traverse and initialize each independent sub-dag
    static_cast<DagGroup&>(root_dag).updateGroupIndex();
    InitDagForDecision(*(item.second));
    m_max_mul_dep =
        std::max(m_max_mul_dep, item.second->m_after_reduction_depth);
    static_cast<DagGroup&>(root_dag).updateGroupIndex();
    root_dag.setSupportShortInt(item.second->supportShortInt());
    root_dag.m_has_int64 = item.second->m_has_int64;
    root_dag.m_has_double = item.second->m_has_double;
  }
  // std::function<uint32_t(uint32_t, uint32_t)> maxFucUint =
  //     [](uint32_t a, uint32_t b) { return std::max(a, b); };
  // // Batch process sub-dags and find the maximum multiplication depth
  // m_max_mul_dep =
  //     std::transform_reduce(std::begin(m_name2dag), std::end(m_name2dag), 0,
  //                           maxFucUint, [&](std::pair<std::string, DagPtr>
  //                           item) {
  //                             InitDagForDecision(*(item.second));

  //                             return item.second->m_after_reduction_depth;
  //                           });

  // Update the root dag's maximum multiplication depth
  std::string tmp_alo_name;
  // shortint uses the concrete library
  setAloName(root_dag, tmp_alo_name);
  root_dag.m_after_reduction_depth = m_max_mul_dep;
  int de = dePar(tmp_alo_name, root_dag);
  m_libs.emplace_back(std::move(tmp_alo_name));
  static_cast<DagGroup&>(root_dag).updateGroupIndex();
  return de;
}

int AloDecision::dePar(const string& alo_name, Dag& dag) {
  // Determine the handler_ptr to process
  m_fhe_manager->InitAloPtr(alo_name);
  // Compile and process the corresponding algorithm
  return m_fhe_manager->compileAlo(alo_name, dag);
}

int AloDecision::genKeys(Dag& dag) {
  for (const auto& item : m_libs) {
    if (m_fhe_manager == nullptr) {
      throw std::logic_error("gen keys m_fhe_manager null !");
    }

    int gen_ret = m_fhe_manager->generateKeys(item, dag);
    THROW_ON_ERROR(gen_ret, "generateKeys");
  }
  return 0;
}

int AloDecision::encryptInput(const Valuation& inputs, bool replace) {
  if (m_libs.size() > 0)
    return m_fhe_manager->encryptInput(m_libs[0], inputs, replace);
  else {
    throw std::logic_error("libs null !");
  }
}

int AloDecision::executor(Dag& dag) {
  auto dag_rewrite = DagTraversal(dag);
  dag_rewrite.backwardPass(CleanNodeHandler(dag));
  if (m_libs.size() > 0)
    return m_fhe_manager->executor(m_libs[0], dag);
  else {
    throw std::logic_error("libs null !");
  }
}

int AloDecision::getDecryptOutput(Valuation& valuation) {
  if (m_libs.size() > 0)
    return m_fhe_manager->getDecryptOutput(m_libs[0], valuation);
  else {
    throw std::logic_error("libs null !");
  }
}

}  // namespace iyfc