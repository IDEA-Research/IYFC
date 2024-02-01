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
#include <cstdint>
#include <memory>
#include <set>
#include <vector>
#include "util/logging.h"
#include "openfhe.h"
#include "proto/openfhe.pb.h"

namespace iyfc {

/**
 * @class OpenFheCkksPara 
 * @brief Parameters to consider for the CKKS algorithm in the openfhe library.
 */
struct OpenFheCkksPara {
  uint32_t mult_depth;  // Maximum multiplication depth
  std::vector<int> rotations;
  uint32_t batch_size;
  bool need_bootstrapping{false};
  uint32_t scaling_mod_size;
  uint32_t first_mod_size;

  void printPara() {
    LOG(LOGLEVEL::Debug,
        " openfhe ckks param: depth %u, batch_size %u, scaling_mod_size %u, "
        "first_mod_size %u \n rotations : \n",
        mult_depth, batch_size, scaling_mod_size, first_mod_size);
    for (const auto &ro : rotations) printf("%d, ", ro);
    if (need_bootstrapping) printf(" \n will bootstrapping \n ");
  }
};

/**
 * @brief Serialize CKKS parameters.
 */
std::unique_ptr<msg::OpenFheCkksPara> serialize(const OpenFheCkksPara &);
/**
 * @brief Deserialize CKKS parameters.
 */
std::unique_ptr<OpenFheCkksPara> deserialize(const msg::OpenFheCkksPara &);


/**
 * @class OpenFheBfvPara 
 * @brief Parameters to consider for the BFV algorithm in the openfhe library.
 */
struct OpenFheBfvPara {
  uint32_t mult_depth;  // Maximum multiplication depth
  std::vector<int> rotations;
  uint32_t batch_size;
  uint32_t plain_modulus;
  uint32_t scaling_mod_size;

  void printPara() {
    LOG(LOGLEVEL::Debug,
        " openfhe bfv param: depth %u, batch_size %u,  plain_modulus %u, scaling_mod_size %u, "
        "rotations : \n",
        mult_depth, batch_size, plain_modulus, scaling_mod_size);
    for (const auto &ro : rotations) printf("%d, ", ro);
  }
};

/**
 * @brief Serialize BFV parameters.
 */
std::unique_ptr<msg::OpenFheBfvPara> serialize(const OpenFheBfvPara &);
/**
 * @brief Deserialize BFV parameters.
 */
std::unique_ptr<OpenFheBfvPara> deserialize(const msg::OpenFheBfvPara &);


}  // namespace iyfc
