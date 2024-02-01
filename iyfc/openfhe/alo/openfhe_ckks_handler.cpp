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
#include "openfhe_ckks_handler.h"
#include "daghandler/ckks_rotation_keys_handler.h"
#include "daghandler/mult_depth_cnt.h"
#include "err_code.h"
#include "util/logging.h"

namespace iyfc {

int OpenFheCkksHandler::dagTranspile(Dag &input_dag) {
  return transpile(input_dag);
};

void OpenFheCkksHandler::transform(Dag &dag, NodeMap<DataType> &types,
                                   NodeMapOptional<std::uint32_t> &scales) {
  auto dag_rewrite = DagTraversal(dag);
  // Deduce types
  dag_rewrite.forwardPass(TypeHandler(dag, types));

  // Pre-calculate constant data nodes
  dag_rewrite.forwardPass(ConstantDoubleHandler(dag, scales));
  // Add encode nodes
  dag_rewrite.forwardPass(TypeHandler(dag, types));
  dag_rewrite.forwardPass(PlaintextInserter(dag, types, scales));
  // Deduce types
  dag_rewrite.forwardPass(TypeHandler(dag, types));

}

void OpenFheCkksHandler::extractSignature(const Dag &dag) {
  std::unordered_map<std::string, OpenFheEncodingInfo> inputs;
  for (auto &input : dag.getInputs()) {
    DataType type = input.second->get<TypeAttr>();
    assert(type != DataType::Undef);
    inputs.emplace(input.first, OpenFheEncodingInfo(type));
  }
  m_signature = std::make_shared<OpenFheSignature>(
      OpenFheSignature(dag.getVecSize(), std::move(inputs)));
}

void OpenFheCkksHandler::determineParameters(Dag &dag,
                                             NodeMap<DataType> types) {
  auto dag_traverse = DagTraversal(dag);
  m_enc_params = std::make_shared<OpenFheCkksPara>();
  m_enc_params->mult_depth = dag.m_after_reduction_depth + 1; // Ensure correctness, set +1 here
  // Rotation parameters
  RotationKeys rks(dag, types);
  dag_traverse.forwardPass(rks);
  std::set<int> set_rokeys = rks.getRotationKeys();
  m_enc_params->rotations.assign(set_rokeys.begin(), set_rokeys.end());

  // Batch size
  m_enc_params->batch_size = dag.getVecSize();
  // Modulus size
  m_enc_params->scaling_mod_size = dag.m_scale-1;  // float * 2**59
  m_enc_params->first_mod_size = dag.m_scale;

  if (m_enc_params->mult_depth > MAX_MULT_DEPTH_NO_BOOT) {
    // To-do: bootstrapping for precision issues
    // m_enc_params->need_bootstrapping = true;
    // dag.m_enable_bootstrap = true;
  }

  m_enc_params->printPara();
}

int OpenFheCkksHandler::transpile(Dag &input_dag) {

  NodeMap<DataType> types(input_dag);
  // Keep scales, manual optimization logic for openfhe will be developed later (to-do)
  NodeMapOptional<std::uint32_t> scales(input_dag);
  // Transform
  transform(input_dag, types, scales);
  // Determine parameters
  determineParameters(input_dag, types);
  // Determine signature
  extractSignature(input_dag);

  return 0;
}

}  // namespace iyfc