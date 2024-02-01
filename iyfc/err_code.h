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
#pragma once
#include <string>
namespace iyfc {

#ifndef RETURN_ON_ERROR
#define RETURN_ON_ERROR(func_ret, call_func_name)              \
  if (func_ret != 0) {                                         \
    printf("%s fail! func_ret %d ", call_func_name, func_ret); \
    return func_ret;                                           \
  }
#endif

#ifndef THROW_ON_ERROR
#define THROW_ON_ERROR(func_ret, call_func_name)               \
  if (func_ret != 0) {                                         \
    throw std::logic_error(std::string(call_func_name) + "fail ! ret = " + std::to_string(func_ret)); \
  }
#endif

// Define function return int error codes
/*
Handle exceptions in the form of error codes for easy collaboration and debugging.
Avoiding the use of exceptions to prevent the calling party from not handling exceptions, leading to process exit.
ToDo:
1.Consider changing to throwing exceptions in future open-source versions.
2.return nullptr changed to throw exception
*/

enum {
  SUCCESS = 0,

  // 11 - 20 build dag errors. Check build parameters/deserialization of the dag source for issues.
  // It may not be compiled or deserialized incorrectly.
  ALOINFO_NULL = -10,           // Algorithm for decision is null
  ALO_DEC_MANAGER_NULL = -11,   // Decision management class is null
  FHE_MANAGER_PARA_NULL = -12,  // Parameters for decision algorithm are null
  DATA_TYPE_NOT_SUPPORT = -13,  // Unsupported data type

  // Input setting errors -21 -

  // 31-40 Output setting errors -31
  OUTPUT_EXPR_NULL = -31,  // Output needs to be bound with the expression

  // 41-60 Encryption-related errors.
  // Check if the dag is compiled or deserialized, and if encryption information is available.
  ENCRPT_ALOINFO_NULL = -41,  // Encryption algorithm information is null
  ENCRPT_LIB_NULL = -42,      // Encryption algorithm library is null
  ENCREPT_CTX_NULL = -43,     // Encryption environment needed is null

  // 61-80 Decryption errors.
  // Check if the dag is compiled or deserialized, and if decryption information is available.
  DECRYPT_ALOINFO_NULL = -61,  // Decryption algorithm information is null
  DECRYPT_LIB_NULL = -62,      // Decryption dag library is null
  DECRYPT_RESULT_NULL = -63,   // Decryption result is null
  DECRYPT_SECRET_NULL = -64,   // Decryption environment is null

  // 101-300 Serialization-related errors.
  // See English description for specific reasons. Basically it will show why it goes wrong.
  LOAD_ALO_OPENFILE_ERR = -101,
  LOAD_ALO_PARSE_MSG_ERR = -102,
  SAVE_INPUT_TOFILE_ALO_NULL = -103,
  SAVE_INPUT_TOFILE_FILE_NOT_OPEN = -104,
  SAVE_INPUT_TOFILE_SERIALIZE_MSG_ERR = -105,
  LOAD_INPUT_FROMFILE_ALO_NULL = -106,
  LOAD_INPUT_FROMFILE_OPEN_ERR = -107,
  LOAD_INPUT_FROMFILE_PARSE_MSG_ERR = -108,
  SAVE_OUTPUT_FILE_ALO_NULL = -109,
  SAVE_OUTPUT_FILE_SERIALIZE_MSG_ERR = -110,
  LOAD_OUTPUT_FILE_ALO_NULL = -111,
  LOAD_OUTPUT_FILE_NOT_OPEN = -112,
  LOAD_OUTPUT_FILE_PARSE_MSG_ERR = -113,
  SAVE_ALO_STR_SERIALIZE_ERR = -114,
  LOAD_ALO_STR_PARSE_MSG_ERR = -115,
  SAVE_INPUT_STR_ALO_NULL = -116,
  SAVE_INPUT_STR_SERIALIZE_ERR = -117,
  LOAD_INPUT_STR_ALO_NULL = -118,
  LOAD_INPUT_STR_PARSE_ERR = -119,
  SAVE_OUTPUT_STR_ALO_NULL = -120,
  SAVE_OUTPUT_STR_SERIALIZE_ERR = -121,
  LOAD_OUTPUT_STR_ALO_NULL = -122,
  LOAD_OUTPUT_STR_PARSE_ERR = -123,
  SAVE_OUTPUT_FILE_NOT_OPEN = -124,
  SAVE_ALO_TO_FILE_ERR = -125,
  SERIALIZE_ALO_MSG_ERR = -126,
  LOAD_INVALID_UNKNOWN_ATTR = -130,
  LOAD_INVALID_ATTR = -131,
  SER_SEAL_NEED_GENKEY_BUT_CKKS_PARA_NULL = -132,
  SER_SEAL_NEED_SIG_BUT_NULL = -133,
  SER_SEAL_NEED_PUBLIC_BUT_NULL = -134,
  SER_SEAL_NEED_SECRET_BUT_NULL = -135,
  SER_SEAL_INFO_TO_STR_ERR = -136,
  DESER_SEAL_INFO_PARSE_ERR = -137,
  SER_SEAL_INPUT_VALUEATION_NULL = -137,
  SER_SEAL_INPUT_SERIALIZETOSTRING_FUC_ERR = -139,
  DESER_PARSEFROMSTRING_FUC_ERR = -140,
  SER_SEAL_OUTPUT_VALUATION_NULL = -141,
  SER_SEAL_OUTPUT_SERIALIZETOSTRING_FUC_ERR = -142,
  DESER_OUTPUT_PARSEFROMSTRING_FUC_ERR = -143,
  LOAD_ALO_ITEM_EMPTY = -144,
  LOAD_INPUT_MSG_EMPTY = -146,
  LOAD_OUTPUT_MSG_EMPTY = -147,
  SER_CONCRETE_PUBLIC_CTX_NULL = -148,
  SER_CONCRETE_SECRET_CTX_NULL = -150,
  SER_CONCRETE_SerializeToString_FUNC_ERR = -151,
  SER_CONCRETE_INPUT_VAL_NULL = -153,
  SER_CONCRETE_INPUT_SerializeToString_ERR = -154,
  DESER_CONCRETE_ParseFromString_FUNC_ERR = -155,
  SER_CONCRETE_OUPUT_VAL_NULL = -156,
  SER_CONCRETE_OUPUT_SerializeToString_ERR = -157,
  DESER_CONCRETE_OUTPUT_ParseFromString_FUNC_ERR = -158,

  SER_SEAL_NEED_GENKEY_BUT_BFV_PARA_NULL = -180,
  SER_SEAL_BFV_NEED_SIG_BUT_NULL = -181,
  SER_SEAL_BFV_NEED_PUBLIC_BUT_NULL = -182,
  SER_SEAL_BFV_NEED_SECRET_BUT_NULL = 183,
  SER_SEAL_BFV_INFO_TO_STR_ERR = -184,
  DESER_SEAL_BFV_INFO_PARSE_ERR = -185,
  SER_SEAL_BFV_INPUT_VALUEATION_NULL = -186,
  SER_SEAL_BFV_INPUT_SERIALIZETOSTRING_FUC_ERR = -187,
  SER_SEAL_BFV_OUTPUT_VALUATION_NULL = -188,
  SER_SEAL_BFV_OUTPUT_SERIALIZETOSTRING_FUC_ERR = -189,

  SER_DAG_ERR = -200,
  SER_ALO_ERR = -201,

  // 550 - 600 Underlying SEAL library.
  // Check if the dag is compiled or deserialized and if the parameter information is available.
  SEAL_CKKS_PARA_NULL = -550,                // CKKS parameters are null
  SEAL_ENCRPT_PARA_NULL = -551,              // Parameters required for encryption are null
  SEAL_ENCRYPT_EMPTY_RESULT = -552,          // Empty encryption result
  SEAL_DECRYPT_RESULT_EMPTY = -553,          // Empty decryption result
  SEAL_EXE_CTX_NULL = -554,                  // Execution environment is null
  SEAL_SECUITY_LEVEL_BITS_NOT_MATCH = -555,  // Security level and bit settings do not match
  SEAL_INPUT_SCALE_NOT_SET = -556,           // Input scale is not set
  SEAL_CONST_SCALE_NOT_SET = -557,           // Constant scale is not set
  SEAL_ENCODE_RAW_ERR = -558,                // Encoding of constant failed
  SEAL_GET_ENCODE_ATTR_ERR = -559,           // Failed to get the encoding attribute value

  SEAL_BFV_PARA_NULL = -580,  // SEAL BFV algorithm parameter error

  // 601 - 650 Underlying library concrete
  CONCRETE_GET_OUTPUTS_VALUE_NULL = -601,  // Output is null
  CONCRETE_SECRET_CTX_NULL = -602,  // Encryption or decryption environment is null (concrete both input secret)
  CONCRETE_PUBLIC_CTX_NULL = -603,  // Execution environment

  // 700 - 800 OpenFHE
  OPENFHE_CKKS_PARA_NULL = -700,
  OPENFHE_ENCRPT_PARA_NULL = -701,
  OPENFHE_ENCRYPT_EMPTY_RESULT = -702,
  OPENFHE_DECRYPT_RESULT_EMPTY = -703,
  OPENFHE_EXE_CTX_NULL = -704,

  SER_OPENFHE_NEED_GENKEY_BUT_CKKS_PARA_NULL = -720,
  SER_OPENFHE_NEED_SIG_BUT_NULL = -721,
  SER_OPENFHE_NEED_PUBLIC_BUT_NULL = -722,
  SER_OPENFHE_NEED_SECRET_BUT_NULL = -723,
  SER_OPENFHE_INFO_TO_STR_ERR = -724,
  DESER_OPENFHE_INFO_PARSE_ERR = -725,
  SER_OPENFHE_INPUT_VALUEATION_NULL = -726,
  SER_OPENFHE_INPUT_SERIALIZETOSTRING_FUC_ERR = -727,
  SER_OPENFHE_OUTPUT_VALUATION_NULL = -729,
  SER_OPENFHE_OUTPUT_SERIALIZETOSTRING_FUC_ERR = -730,

  OPENFHE_BFV_PARA_NULL = -740,
  SER_OPENFHE_NEED_GENKEY_BUT_BFV_PARA_NULL = -741,

  // Comparison example errors
  CMP_ERR_OUTPUT = -1001,  // Comparison example output error
  CMP_NUM_LIMIT = -1002,   // Comparison limits the number of elements to 16

  SORT_NUM_LIMIT = -1051,   // Sorting limits the number of elements to 16
  SORT_ERR_OUTPUT = -1052,  // Sorting output error

  FFT_ERR_REAL_OUTPUT = -1100,       // FFT real part retrieval error
  FFT_ERR_IMAG_OUTPUT = -1101,       // FFT imaginary part retrieval error
  FFT_ERR_IMAG_OUTPUT_SIZE = -1102,  // FFT returned data size error

};
}  // namespace iyfc
