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

#include "alo_register.h"

namespace iyfc {

SealCkksAlo::SealCkksAlo() : AloHandler() {
  // Separated into two steps first, later check if other algorithm flows need to be merged
  m_parameters = std::make_shared<SealCkksHandler>();
  m_encrypt = std::make_shared<SealCkksAdapter>();
};

SealBfvAlo::SealBfvAlo() : AloHandler() {
  // Different parameter processing than ckks
  m_parameters = std::make_shared<BfvParametersHandler>();
  m_encrypt = std::make_shared<SealBfvAdapter>();
};


ConcreteAlo::ConcreteAlo() : AloHandler() {
  m_parameters = std::make_shared<ConcreteParametersHandler>();
  m_encrypt = std::make_shared<ConcreteAdapter>();
};


OpenfheCkksAlo::OpenfheCkksAlo() : AloHandler() {
  m_parameters = std::make_shared<OpenFheCkksHandler>();
  m_encrypt = std::make_shared<OpenFheCkksAdapter>();
};

OpenfheBfvAlo::OpenfheBfvAlo() : AloHandler() {
  m_parameters = std::make_shared<OpenFheBfvHandler>();
  m_encrypt = std::make_shared<OpenFheBfvAdapter>();
};

}  // namespace iyfc