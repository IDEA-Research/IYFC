
/*
 * Calling Rust Function Interface from C, with the Specific Implementation in the Rust Language Library
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
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
extern "C" {
struct ClientKey;
struct ServerKey;
struct FheUint4;

//Calling Rust Function Interface from C, with the Specific Implementation in the Rust Language Library

ClientKey* c_generate_client_key();
void c_free_clientkey(ClientKey* p_keys);

ServerKey* c_generate_server_key(const ClientKey* p_keys);
void c_free_serverkey(ServerKey* p_keys);

FheUint4* c_try_encrypt(const ClientKey* p_keys, unsigned int a);
void c_free_fheuint2(FheUint4 * p);

FheUint4* c_executor_div(const ServerKey* p_keys, FheUint4* lhs, FheUint4* rhs);
FheUint4* c_executor_div_plain(const ServerKey* p_keys, FheUint4* lhs, unsigned int rhs);
FheUint4* c_executor_plain_div(const ServerKey* p_keys, unsigned int lhs, FheUint4* rhs);

FheUint4* c_executor_add(const ServerKey* p_keys, FheUint4* lhs, FheUint4* rhs);
FheUint4* c_executor_add_plain(const ServerKey* p_keys, FheUint4* lhs, unsigned int rhs);

FheUint4* c_executor_sub(const ServerKey* p_keys, FheUint4* lhs, FheUint4* rhs);
FheUint4* c_executor_sub_plain(const ServerKey* p_keys, FheUint4* lhs, unsigned int rhs);
FheUint4* c_executor_plain_sub(const ServerKey* p_keys, unsigned int lhs, FheUint4* rhs);

FheUint4* c_executor_mul(const ServerKey* p_keys, FheUint4* lhs, FheUint4* rhs);
FheUint4* c_executor_mul_plain(const ServerKey* p_keys, FheUint4* lhs, unsigned int rhs);

FheUint4* c_left_rotate(const ServerKey* p_keys, FheUint4* lhs, unsigned int rhs);
FheUint4* c_right_rotate(const ServerKey* p_keys, FheUint4* lhs, unsigned int rhs);
FheUint4* c_negate(const ServerKey* p_keys, FheUint4* lhs);



unsigned int c_decrypt(const ClientKey* p_keys, FheUint4* result);

void c_free_cstring(char * p);

char * c_serialize_client_key(const ClientKey* p_keys);
ClientKey* c_deserialize_client_key(char * input);

char * c_serialize_server_key(const ServerKey * p_keys);
ServerKey * c_deserialize_server_key(char * input);

char * c_serialize_fheuint2(const FheUint4 * lhs);
FheUint4 * c_deserialize_fheuint2(char * input);

}