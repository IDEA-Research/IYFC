
/*
 * C Calling Rust Test
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

#include<stdio.h>
#include<stdlib.h>
#include<string>
#include <cstring>
#include"concrete_header.h"


int main(){
    ClientKey *client_key = c_generate_client_key();
    printf("finish c_generate_client_key\n");
    ServerKey *server_key = c_generate_server_key(client_key);
    printf("finish c_generate_server_key\n");

    char * char_str = c_serialize_client_key(client_key);
    //printf("char_str %s \n", char_str);
    ClientKey *client_key_de_se = c_deserialize_client_key(char_str);

    FheUint4 *lhs1 = c_try_encrypt(client_key_de_se, 2);
    FheUint4 *rhs2 = c_try_encrypt(client_key_de_se, 1);

    char * fhelhs1 = c_serialize_fheuint2(lhs1);
    FheUint4 * lhs_de = c_deserialize_fheuint2(fhelhs1);

    printf("finish c_deserialize_fheuint2 %s \n", fhelhs1);
    FheUint4 * en_result = c_executor_div(server_key,lhs_de,rhs2);
    printf("finish c_executor\n");
    printf("finis c_decrypt result: %d \n", c_decrypt(client_key_de_se,en_result));
    FheUint4 * en_result_mul = c_executor_mul(server_key,lhs1,rhs2);
    //printf("finish c_executor_mul\n");
    //printf("finis c_decrypt result: %d \n", c_decrypt(client_key_de_se,en_result_mul));
    return 0;
}