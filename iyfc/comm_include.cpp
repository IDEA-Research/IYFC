
/*
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

#include "comm_include.h"

namespace iyfc {

DagSerializePara::DagSerializePara(bool node_info, bool gen_key, bool sig, bool exe_ctx,
                                   bool encrpt_ctx, bool decrypt_ctx)
    : need_node_info(node_info),
      need_genkey_info(gen_key),
      need_sig_info(sig),
      need_exe_ctx(exe_ctx),
      need_encrpt_ctx(encrpt_ctx),
      need_decrypt_ctx(decrypt_ctx) {}

  void DagSerializePara::set_need_node(bool par){
    need_node_info = par;

  }
  void DagSerializePara::set_need_genkey(bool par){
    need_genkey_info = par;
  }
  void DagSerializePara::set_need_sig(bool par){
    need_sig_info = par;
  }
  void DagSerializePara::set_need_exe_ctx(bool par){
    need_exe_ctx = par;
  }
  void DagSerializePara::set_need_encrpt_ctx(bool par){
    need_encrpt_ctx = par;
  }
  void DagSerializePara::set_need_decrypt_ctx(bool par){
    need_decrypt_ctx = par;
  }


}  // namespace iyfc