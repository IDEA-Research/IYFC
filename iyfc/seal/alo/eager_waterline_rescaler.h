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
#include "rescaler.h"

namespace iyfc {
/**
 * @class EagerWaterlineRescaler
 * @brief Strategy for adding rescaler nodes, inspired by EVA. Rescales only when exceeding a specified waterline.
 */
class EagerWaterlineRescaler : public Rescaler {
  std::uint32_t m_min_scale;
  std::uint32_t m_fixed_rescale{DEFAULT_SCALE};

 public:

  /**
   * @brief EagerWaterlineRescaler constructor.
   * @param g The DAG.
   * @param type NodeMap representing node data types.
   * @param scale NodeMapOptional representing node scales.
   */
  EagerWaterlineRescaler(Dag &g, NodeMap<DataType> &type,
                         NodeMapOptional<std::uint32_t> &scale);

  virtual ~EagerWaterlineRescaler() {}

  /**
   * @brief Operator overloading for traversing the DAG and inserting rescale nodes where needed.
   */
  void operator()(NodePtr &node);
};

}  // namespace iyfc
