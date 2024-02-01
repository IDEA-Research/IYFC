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
#include "seal_ckks_handler.h"
#include <seal/util/hestdparms.h>
#include "err_code.h"
#include "util/logging.h"

namespace iyfc {
int SealCkksHandler::dagTranspile(Dag &input_dag) { return transpile(input_dag); };

void SealCkksHandler::transform(Dag &dag, NodeMap<DataType> &types,
                                NodeMapOptional<std::uint32_t> &scales) {
  auto dag_rewrite = DagTraversal(dag);
  // Type inference pass
  dag_rewrite.forwardPass(TypeHandler(dag, types));
  // Precompute constant data nodes
  dag_rewrite.forwardPass(ConstantDoubleHandler(dag, scales));
  // rescale
  dag_rewrite.forwardPass(EagerWaterlineRescaler(dag, types, scales));
  LOG(LOGLEVEL::Debug, "Running EagerWaterlineRescaler pass");

  // Type inference pass
  dag_rewrite.forwardPass(TypeHandler(dag, types));

  // Insert encode nodes
  dag_rewrite.forwardPass(EncodeInserter(dag, types, scales));

  // Type inference pass
  dag_rewrite.forwardPass(TypeHandler(dag, types));

  // Relinearization pass
  dag_rewrite.forwardPass(LazyRelinearizer(dag, types, scales));

  // Type inference pass
  dag_rewrite.forwardPass(TypeHandler(dag, types));

  // Mod-switch strategy
  dag_rewrite.backwardPass(ModSwitcher(dag, types, scales));

  // Type inference pass
  dag_rewrite.forwardPass(TypeHandler(dag, types));

  // SEAL lowering pass
  dag_rewrite.forwardPass(SEALLowering(dag, types));
}

void SealCkksHandler::validate(Dag &dag, NodeMap<DataType> &types,
                               NodeMapOptional<std::uint32_t> &scales) {
  auto dag_traverse = DagTraversal(dag);

  // update types
  dag_traverse.forwardPass(TypeHandler(dag, types));
  LevelsChecker lc(dag, types);
  dag_traverse.forwardPass(lc);
  try {
    LOG(LOGLEVEL::Debug, "Running ParameterChecker pass");
    ParameterChecker pc(dag, types);
    dag_traverse.forwardPass(pc);
  } catch (const InconsistentParameters &e) {
    warn(
        "The current rescaler produced inconsistent parameters. This is a "
        "bug, as this rescaler should be able to handle all programs.");
  }
  LOG(LOGLEVEL::Debug, "Running ScalesChecker pass");
  ScalesChecker sc(dag, scales, types);
  dag_traverse.forwardPass(sc);
}

std::size_t SealCkksHandler::getMinDegreeForBitCount(
    int (*MaxBitsFun)(std::size_t), int bit_cnt) {
  std::size_t degree = 1024;
  int max_bits_seen = 0;
  while (true) {
    auto max_bits_for_degree = MaxBitsFun(degree);
    max_bits_seen = std::max(max_bits_seen, max_bits_for_degree);
    //==0 Failed to check parameter table
    if (max_bits_for_degree == 0) {
      warn(
          "Dag requires a %u bit modulus, but parameters are available for a "
          "maximum of %u ",
          bit_cnt, max_bits_seen);
      throw std::logic_error("in seal ckks, err bit modulus!");
    }
    if (max_bits_for_degree >= bit_cnt) {
      return degree;
    }
    degree *= 2;
  }
}

void SealCkksHandler::extractSignature(const Dag &dag) {
  std::unordered_map<std::string, SealEncodingInfo> inputs;

  for (auto &input : dag.getInputs()) {
    DataType type = input.second->get<TypeAttr>();
    assert(type != DataType::Undef);

    inputs.emplace(
        input.first,
        SealEncodingInfo(type, input.second->get<EncodeAtScaleAttr>(),
                         input.second->get<EncodeAtLevelAttr>()));
  }
  m_signature = std::make_shared<SealSignature>(
      SealSignature(dag.getVecSize(), std::move(inputs)));
}

int SealCkksHandler::determineEncryptionParameters(
    Dag &dag, NodeMapOptional<std::uint32_t> &scales, NodeMap<DataType> types) {
  auto dag_traverse = DagTraversal(dag);
  EncryptionParametersSelector eps(dag, scales, types);
  dag_traverse.forwardPass(eps);
  // Rotate key selector
  RotationKeys rks(dag, types);
  dag_traverse.forwardPass(rks);
  // LOG(LOGLEVEL::Debug, "\n after EncryptionParametersSelector %s \n",
  // dag.toDOT().c_str());

  m_enc_params = std::make_shared<CKKSParameters>();
  m_enc_params->prime_bits = eps.getEncryptionParameters();
  m_enc_params->rotations = rks.getRotationKeys();

  // If bit_cnt is too large, parameter acquisition will fail. max 881
  int bit_cnt = 0;
  for (auto &log_q : m_enc_params->prime_bits) bit_cnt += log_q;

  // Decisions based on different parameters of security intensity
  if (m_config.m_security_level <= 128) {
    if (m_config.m_quantum_safe)
      m_enc_params->poly_modulus_degree = getMinDegreeForBitCount(
          &seal::util::seal_he_std_parms_128_tq, bit_cnt);
    else
      m_enc_params->poly_modulus_degree = getMinDegreeForBitCount(
          &seal::util::seal_he_std_parms_128_tc, bit_cnt);
  } else if (m_config.m_security_level <= 192) {
    if (m_config.m_quantum_safe)
      m_enc_params->poly_modulus_degree = getMinDegreeForBitCount(
          &seal::util::seal_he_std_parms_192_tq, bit_cnt);
    else
      m_enc_params->poly_modulus_degree = getMinDegreeForBitCount(
          &seal::util::seal_he_std_parms_192_tc, bit_cnt);
  } else if (m_config.m_security_level <= 256) {
    if (m_config.m_quantum_safe)
      m_enc_params->poly_modulus_degree = getMinDegreeForBitCount(
          &seal::util::seal_he_std_parms_256_tq, bit_cnt);
    else
      m_enc_params->poly_modulus_degree = getMinDegreeForBitCount(
          &seal::util::seal_he_std_parms_256_tc, bit_cnt);
  } else {
    warn(
        "iyfc has support for up to 256 bit security, but %d bit security was "
        "requested.",
        m_config.m_security_level);

    return SEAL_SECUITY_LEVEL_BITS_NOT_MATCH;
  }

  auto slots = m_enc_params->poly_modulus_degree / 2;
  if (m_config.m_warn_vecsize && slots > dag.getVecSize()) {
    LOG(LOGLEVEL::Debug,
        "Dag specifies vector size %i while at least %i slots are "
        "required for security. "
        "This does not affect correctness, as the smaller vector size will "
        "be transparently emulated. "
        "However, using a vector size up to %i would come at no additional "
        "cost.",
        dag.getVecSize(), slots, slots);
  }
  if (slots < dag.getVecSize()) {
    if (m_config.m_warn_vecsize) {
      LOG(LOGLEVEL::Debug,
          "Dag uses vector size %i while only %i slots are required for "
          "security. "
          "This does not affect correctness, but higher performance may be "
          "available "
          "with a smaller vector size.",
          dag.getVecSize(), slots);
    }
    m_enc_params->poly_modulus_degree = 2 * dag.getVecSize();
  }

  if (logLevelLeast(LOGLEVEL::Debug)) {
    printf("IYFC: Encryption parameters for %s are:\n  Q = [",
           dag.getName().c_str());
    bool first = true;
    for (auto &log_q : m_enc_params->prime_bits) {
      if (first) {
        first = false;
        printf("%i", log_q);
      } else {
        printf(",%i", log_q);
      }
    }
    int n = m_enc_params->poly_modulus_degree;
    int nexp = 0;
    while (n >>= 1) ++nexp;
    printf(
        "] (total bits %i)\n  N = 2^%i (available slots %i)\n  Rotation "
        "keys: ",
        bit_cnt, nexp, m_enc_params->poly_modulus_degree / 2);
    first = true;
    for (auto &rotation : m_enc_params->rotations) {
      if (first) {
        first = false;
        printf("%i", rotation);
      } else {
        printf(", %i", rotation);
      }
    }
    printf(" (count %lu)\n", m_enc_params->rotations.size());
  }

  return 0;
}

int SealCkksHandler::transpile(Dag &input_dag) {
  /*
  printf("Compiling %s for CKKS with:\n%s \n", input_dag.getName().c_str(),
         m_config.toString(2).c_str());
  */
  // Process the uint32 node and convert it to a seal vector const node
  auto dag_rewrite = DagTraversal(input_dag);
  // types_map scales update values with optimization strategy
  NodeMap<DataType> types(input_dag);
  NodeMapOptional<std::uint32_t> scales(input_dag);

  for (auto &source : input_dag.getSources()) {
    // Error out if the scale attribute doesn't exist
    if (!source->has<EncodeAtScaleAttr>()) {
      for (auto &entry : input_dag.getInputs()) {
        if (source == entry.second) {
          warn("The scale for input %s was not set.", entry.first.c_str());
          return SEAL_INPUT_SCALE_NOT_SET;
        }
      }
      warn("The scale for a constant was not set.");
      return SEAL_CONST_SCALE_NOT_SET;
    }

    try {
      scales[source] = source->get<EncodeAtScaleAttr>();
    } catch (char const *msg) {
      throw ::std::logic_error("get EncodeAtScaleAttr err ");
    }
  }

  // transform
  transform(input_dag, types, scales);

  MultDepthCnt depth(input_dag, types);
  dag_rewrite.forwardPass(depth);
  LOG(LOGLEVEL::Debug, " in seal after transform depth %u ",
      input_dag.m_after_reduction_depth);
  // validate
  validate(input_dag, types, scales);
  // determine encryption parameters
  int para_ret = determineEncryptionParameters(input_dag, scales, types);
  THROW_ON_ERROR(para_ret, "determineEncryptionParameters");

  extractSignature(input_dag);

  LOG(LOGLEVEL::Debug, " after SealCkksHandler transpile");

  return 0;
}

}  // namespace iyfc