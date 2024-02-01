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
#include "examples.h"
using namespace iyfc;
using namespace std;

/**
 * @brief Example demonstrating the use of a sorting computation DAG.
 */
void example_sort() {
  // Step 1: Build a comparison computation DAG (not serialized here)
  DagPtr dag = buildSortDag("sort_example");
  compileDag(dag);
  genKeys(dag);

  vector<uint32_t> vec_org{8, 7,  6,  5,  4,  3,  2,  1,
                           9, 10, 11, 12, 13, 14, 15, 16};

  Valuation inputs;
  encodeOrgInputforSort(vec_org, inputs);
  encryptInput(dag, inputs);
  exeDag(dag);
  std::vector<std::vector<uint32_t>> matrix_result;
  uint32_t num_cnt = vec_org.size();
  getSortOutputs(dag, num_cnt, matrix_result);

  for (const auto& vec_item : matrix_result) {
    printf("\n");
    for (const auto& item : vec_item) {
      printf("%d ", item);
    }
  }
  // Calculate Hamming distance for each element
  vector<uint32_t> vec_hansum;
  vec_hansum.resize(num_cnt);
  for (uint32_t i = 0; i < num_cnt; i++) {
    for (uint32_t j = 0; j < num_cnt; j++) {
      // Sum each column in the matrix to get the Hamming weight for each number
      vec_hansum[i] += matrix_result[j][i];
    }
  }
  printf("\n hansum \n");
  for (const auto& item : vec_hansum) {
    printf("%u ", item);
  }
  printf("\n");
  releaseDag(dag);

}