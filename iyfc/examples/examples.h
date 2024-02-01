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
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <climits>
#include <memory>
#include <mutex>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "comm_include.h"
#include "err_code.h"
#include "iyfc_include.h"
#include "util/math_util.h"
#include "util/overloaded.h"
#include "util/timer.h"
#include "dag/expr.h"

using namespace std;
using namespace iyfc;

void example_simple_ckks();

void example_simple_bfv();

void example_psi_bfv();

void example_sort();

void example_div();

void example_div_v2();

void example_serialization();

void example_query();

void example_group();

void example_avg();

void example_query_index();

void example_query_cnt();

void example_query_sum();



template <typename T>
inline void print_vector(std::vector<T> vec, std::size_t print_size = 100) {
  size_t size_print = std::min(vec.size(), print_size);
  std::cout << std::fixed << std::setprecision(3);
  std::cout << "vec:    [";
  for (std::size_t i = 0; i < size_print; i++) {
    std::cout << " " << vec[i] << ((i != size_print - 1) ? "," : " ]\n");
  }
}


inline void printValuation(const Valuation &val, size_t max_print_size = 100) {
  for (const auto &item : val) {
    const auto &obj = item.second;
    std::visit(
        Overloaded{
            [&](const std::vector<double> &v) { print_vector(v, max_print_size); },
            [&](const std::vector<int64_t> &v) { print_vector(v,max_print_size); },
            [&](const double &v) { std::cout << "double value: " << v << "\n"; },
            [&](const int64_t &v) { std::cout << "int64_t value: " << v << "\n"; },
            [&](const uint8_t &v) { std::cout << " uint8_t value: " << v << "\n"; }},
        obj);
  }
}

