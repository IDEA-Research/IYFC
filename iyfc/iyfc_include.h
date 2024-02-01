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
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include "dag/expr.h"
#include "err_code.h"
#include "comm_include.h"
namespace iyfc {

/**
 * @brief      Step1: Create a DAG for custom expression calculation.
 * @details    This function initializes a new DAG with the given name and size.
 *
 * @param[in]  str_name The name of the DAG.
 * @param[in]  u_size   vec size (default:1024). 
 * It can be chosen by the user, and it may be automatically adjusted during the compilation process.
 *
 * @return     DagPtr The pointer to the newly created DAG.
 */
DagPtr initDag(const std::string& str_name, uint32_t u_size = 1024);

/**
 * @brief      Create a new DagGroup.
 * @details    This function initializes a new DagGroup with the given name and size, constructing multiple sub-DAGs.
 *
 * @param[in]  str_name  The name of the DagGroup.
 * @param[in]  u_size    The size of the DagGroup (default:1024).
 *
 * @return     DagPtr    The pointer to the newly created DagGroup.
 */
DagPtr initDagGroup(const std::string& str_name, uint32_t u_size = 1024);

/**
 * @brief      Add a child Dag to a DagGroup.
 * @details    This function adds the given child Dag to the specified DagGroup.
 *
 * @param[in]  group  The target DagGroup that will receive the child Dag.
 * @param[in]  child  The child Dag to be added.
 */
void addDag(DagPtr group, DagPtr child);

/**
 * @brief      Step2: Set the name of input node.
 * @details    Each input node must have a unique name for mapping specific input values to encryption.
 *  Calling this function multiple times can create multiple input nodes.
 *
 * @param[in]  dag_ptr The target Dag where the input name is to be set.
 * @param[in]  name    Custom input node name.
 *
 * @return     Expr    Expression object.
 */
Expr setInputName(DagPtr dag_ptr, const std::string& name);

/*
            Step 3: Build your own expressions
            e1: Support real number vectors + - *
                Expr tmp_expr_1 = Expr(dag, {1.0});// Currently, Seal supports real number vectors
                Expr tmp_expr_2 = Expr(dag, {1.0,2.0}});
                Expr tmp_expr_out = tmp_expr_1 * tmp_expr_2;
                setOutput(dag,"z", tmp_expr_out);

            e2: Currently, uint3 integers are not practical, and this feature is temporarily reserved.
                Division supports uint3 integers + - * /
                Expr tmp_expr_1 = Expr(dag, 6);
                setOutput(dag,"z", tmp_expr_1 / 2);

*/

/**
 * @brief      Step 4: Set the output node name.
 * @details    Each output node must have a unique name for binding to an input expression.
 * Calling this function multiple times can create multiple output nodes.
 *
 * @param[in]  dag_ptr  The target DagPtr where the output node name is to be set.
 * @param[in]  name     Custom output node name.
 * @param[in]  epxr     The expression associated with the output node.
 *
 * @return     int  Error code. Please refer to the err_code explanation for specific error codes.
 */
int setOutput(DagPtr dag_ptr, const std::string& name, const Expr& epxr);

/**
 * @brief      Step 5: Compile and determine which algorithm library and parameters will use.
 * @details    In this step, the specific algorithm library (seal concrete) and corresponding parameters are determined, transparent to the user.
 *
 * @param[in]  dag_ptr  The DAG to be compiled.
 *
 * @return     int  Error code.
 */
int compileDag(DagPtr dag_ptr);

/**
 * @brief      Step 6: Generate key pairs.
 * @details    In this step, public key (public_ctx) and private key (secret_ctx) are generated.
 * According to concrete, these correspond to the public key of the server and the private key of the client, respectively.
 *
 * @param[in]  dag_ptr The DAG for which key pairs need to be generated.
 *
 * @return     int  Error code.
 */
int genKeys(DagPtr dag_ptr);

/**
 * @brief      Step 7: Encrypt Input.
 * @details    With the presence of the public_ctx node, it is possible to encrypt input data.
 * Note: in the concrete implementation, only the client-secret_ctx is capable of performing encryption.
 *
 * @param[in]  dag_ptr  The target DagPtr for which input encryption is to be performed.
 * @param[in]  inputs   Plaintext input data that needs to be encrypted.
 * @param[in]  replace  Whether to replace the input data. By default, it merges the input data.
 *
 * @return     int  Error code.
 */
int encryptInput(DagPtr dag_ptr, const Valuation& inputs, bool replace = false);

/**
 * @brief      Step 8: Execute Computation.
 * @details    With the presence of the public_ctx node, it is possible to execute computational logic and generate ciphertext results.
 * In the concrete implementation, this capability is specific to the server with the public_ctx.
 * @param[in]  dag_ptr     The target Dag, for which computation needs to be executed.
 * @param[in]  set_inputs  Optional parameter. If provided, it indicates that only the program logic associated with the input set should be executed.
 *
 * @return     int  Error code.
 */
int exeDag(DagPtr dag_ptr,
           const std::unordered_set<std::string>& set_inputs = {});

/**
 * @brief      Step 9: Decrypt Output.
 * @details    With the presence of the secret_ctx node, it is possible to execute decryption logic and generate plaintext results.
 * In the concrete implementation, this capability is specific to the client.
 * @param[in]  dag_ptr   The target Dag, for which output decryption needs to be executed.
 * @param[in]  outputs   The reference to the decrypted plaintext results.
 *
 * @return     int  Error code.
 */
int decryptOutput(DagPtr dag_ptr, Valuation& outputs);

/**
 * @brief      Step 10: Release Dag.
 * @details    Call the delete function to destruct and release the Dag.
 * 
 * @param[in]  dag_ptr  The Dag to be released.
 *
 * @return     int  Error code.
 */
int releaseDag(DagPtr dag_ptr);

/**
 * @brief      Set parameters for DAG serialization.
 * @details    Different parameter settings will affect the information included in the serialized DAG.
 * Specifically, note that in the Seal case, encryption is public, decryption is secret, and execution is public.
 * In the Concrete case, both encryption and decryption occur on the client side, while execution is on the server side.
 * This distinction is made to facilitate future extensions.
 * In other words, nodes that Seal can execute also have encryption capabilities.
 *
 * @param[in]  dag_ptr             The DAG for which serialization parameters need to be set.
 * @param[in]  need_node_info      Original DAG information, including nodes and computational logic.
 * @param[in]  need_genkey_info    Parameters for generating keys, required for genkeys.
 * @param[in]  need_sig_info       Signature information, required for encryption.
 * @param[in]  need_exe_ctx        Execution environment, needed for executing computational logic using Seal.
 * @param[in]  need_encrpt_ctx     Encryption environment using Seal.
 * @param[in]  need_decrypt_ctx    Decryption environment using Seal.
 *
 * @return     int  Error code.
 */
int setDagSerializePara(DagPtr dag_ptr, bool need_node_info,
                        bool need_genkey_info, bool need_sig_info,
                        bool need_exe_ctx, bool need_encrpt_ctx,
                        bool need_decrypt_ctx);

/**
 * @brief      Serialize a DAG to a string.
 * @details    After setting serialization parameters using the setDagSerializePara function,
 * use this function to serialize the DAG.
 *
 * @param[in]   dag_ptr  The DAG to be serialized.
 * @param[out]  str_dag  Reference to string to store the serialized DAG string.
 *
 * @return     int  Error code.
 */
int saveDagToStr(DagPtr dag_ptr, std::string& str_dag);

/**
 * @brief      Deserialize a DAG from a string.
 *
 * @param[in]  str_dag  The string containing the serialized DAG.
 *
 * @return     DagPtr   The deserialized DAG.
 */
DagPtr loadDagFromStr(const std::string& str_dag);

/**
 * @brief      Serialize the computation logic nodes of a DagGroup.
 * @details    Serialize the computation logic nodes of a composite DagGroup into a string for future use.
 * The setDagSerializePara function must be called first to set serialization parameters.
 *
 * @param[in]  dag_ptr  The DagGroup to be serialized.
 * @param[out] str_dag  A string reference to store the serialized computation logic nodes of the DagGroup.
 *
 * @return     int      Error code.
 */
int saveGroupNodesToStr(DagPtr dag_ptr, std::string& str_dag);

/**
 * @brief      Deserialize the computation logic nodes of a DagGroup from a string.
 *
 * @param[in]  str_dag  The string containing the serialized computation logic nodes of the DagGroup.
 *
 * @return     DagPtr   The deserialized DagGroup.
 */
DagPtr loadGroupNodesFromStr(const std::string& str_dag);

/**
 * @brief      Retrieve a child DAG by its name.
 *
 * @param[in]  dag_ptr  The DagGroup in which to search for the child DAG.
 * @param[in]  name     The name of the child DAG to retrieve.
 *
 * @return     DagPtr   The specified child DAG.
 */
DagPtr getChildDagByName(DagPtr dag_ptr, const std::string& name);

/**
 * @brief      Serialize keys to a string.
 * @details    After calling setDagSerializePara, you can also independently serialize the information of public_ctx and secret_ctx.
 * 
 * @param[in]   dag_ptr   The DAG for which keys need to be serialized.
 * @param[out]  str_keys  The string containing the serialized key information of the DAG.
 *
 * @return     int  Error code.
 */
int saveKeysInfoToStr(DagPtr dag_ptr, std::string& str_keys);

/**
 * @brief      Deserialize key information into an existing DAG.
 *
 * @param[in]  dag_ptr   The DAG to which key information needs to be deserialized.
 * @param[in]  str_keys  The string containing the serialized key information.
 *
 * @return     int  Error code.
 */
int loadKeysFromStr(DagPtr dag_ptr, const std::string& str_keys);

/**
 * @brief      Serialize input information into the DAG.
 *
 * @param[in]   dag_ptr    The DAG to which input information needs to be serialized.
 * @param[out]  str_input  str_input  The string to store the serialized input information.
 *
 * @return     int  Error code.
 */
int savaInputTostr(DagPtr dag_ptr, std::string& str_input);

/**
 * @brief      Deserialize input information into an existing DAG.
 *
 * @param[in]  dag_ptr        The DAG for which input information needs to be deserialized.
 * @param[in]  str_input      The string containing the serialized input information.
 * @param[in]  replace        Optional boolean parameter indicating whether to replace existing input information. Default is false.
 *
 * @return     int  Error code.
 */
int loadInputFromStr(DagPtr dag_ptr, const std::string& str_input,
                     bool replace = false);

/**
 * @brief      Serialize encrypted output information into a DAG.
 *
 * @param[in]  dag_ptr      The DAG for which encrypted output information is to be serialized.
 * @param[out] str_output   The string to store the serialized encrypted output information.
 *
 * @return     int  Error code.
 */
int savaOutputTostr(DagPtr dag_ptr, std::string& str_output);

/**
 * @brief      Deserialize encrypted output information into an existing DAG.
 *
 * @param[in]  dag_ptr       The DAG for which encrypted output information is to be deserialized.
 * @param[in]  str_output    The string containing the serialized encrypted output information.
 * @return     int  错误码
 */
int loadOutputFromStr(DagPtr dag_ptr, const std::string& str_output);

/**
 * @brief      Preprocesses raw data for comparison scenarios by converting it into a SEAL plaintext vector.
 * 
 * @param[in]  vec_org           The original data as a vector of unsigned integers.
 * @param[in]  input_name        The name associated with the input data.
 * @param[out] inputs            A Valuation reference to store the generated plaintext input.
 *
 * @return     int  Error code. Returns 0 on success, CMP_NUM_LIMIT if the input size exceeds the maximum limit.
 */
int encodeOrgInputforCmp(const std::vector<uint32_t>& vec_org,
                         const std::string& input_name, Valuation& inputs);

/**
 * @brief      Retrieves the comparison results from a DAG.
 *
 * @param[in]  dag_ptr           The DAG from which to retrieve the comparison results.
 * @param[in]  num_cnt           The size of the original comparison data.
 * @param[in]  result_name       The name associated with the result.
 * @param[out] vec_results       A vector to store the comparison results, where 1 indicates a satisfied condition, and 0 indicates an unsatisfied condition.
 *
 * @return     int               Error code. Returns 0 on success, CMP_ERR_OUTPUT if there is an issue with the output.
 */
int getCmpOutputs(DagPtr dag_ptr, uint32_t num_cnt,
                  const std::string& result_name,
                  std::vector<uint32_t>& vec_results);

/**
 * @brief      Preprocesses data for the sorting case by encoding the input array. (lhs)
 * @details    This function encodes the input array by repeating each element to generate an upper matrix.
 * For example, given the input array [1,2,3,4], the resulting upper matrix x will be [1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4].
 *
 * @param[in]  vec_input        The original array to be sorted.
 * @param[out] vec_x            The resulting upper matrix.
 *
 * @return     int              Error code. Returns 0 on success.
 */
int encodeSortUpInput(const std::vector<uint32_t>& vec_input,
                      std::vector<uint32_t>& vec_x);

/**
 * @brief      Preprocesses data for the sorting case by encoding the input array. (rhs)
 * @details    This function encodes the input array by repeating the elements to generate a lower matrix.
 * For example, given the input array [1,2,3,4], the resulting lower matrix y will be [1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4].
 *
 * @param[in]  vec_input        The original array to be sorted.
 * @param[out] vec_y            The resulting lower matrix.
 *
 * @return     int              Error code. Returns 0 on success.
 */
int encodeSortDownInput(const std::vector<uint32_t>& vec_input,
                        std::vector<uint32_t>& vec_y);

/**
 * @brief      Preprocesses original data for the sorting case, constructing inputs for comparison.
 * @details    The essence of sorting is to compare matrices.
 * This function calls encodeSortUpInput and encodeSortDownInput to generate the corresponding Valuation.
 *
 * @param[in]  vec_org           The original array to be sorted.
 * @param[out] inputs            The Valuation containing plaintext inputs for calculations.
 *
 * @return     int               Error code.
 */
int encodeOrgInputforSort(const std::vector<uint32_t>& vec_org,
                          Valuation& inputs);

/**
 * @brief      Builds a sorting DAG for the sorting case.
 *
 * @param[in]  dag_name              The name of the DAG to be constructed.
 *
 * @return     DagPtr                The constructed sorting DAG.
 */
DagPtr buildSortDag(const std::string& dag_name);

/**
 * @brief      Retrieves the sorting comparison matrix results for the sorting case.
 *
 * @param[in]   dag_ptr               The DAG
 * @param[in]   num_cnt               The size of the original data for comparison.
 * @param[out]  vec_results           Sorting results.
 * It represents a num_cnt * num_cnt matrix, where result[i][j] indicates the comparison relationship between the i-th and j-th numbers.
 * 
 * @return      int                   Error code.
 */
int getSortOutputs(DagPtr dag_ptr, uint32_t num_cnt,
                   std::vector<std::vector<uint32_t>>& vec_results);
// sort

/**
 * @brief      Set the scale parameter (default: 60).
 * If the depth precision of the multiplication depth is required to be small, you can set it to a smaller value.
 *
 * @param[in]   dag_ptr               The DAG for which to set the scale parameter.
 * @param[in]   u_scale               The scale parameter to be set.
 */
void setScale(DagPtr dag_ptr, uint32_t u_scale);

// void setOutputRange(DagPtr dag_ptr, uint32_t u_rangle);

/**
 * @brief      Retrieve the result of a specified counter output in the sorting DAG.
 *
 * @param[in]   dag_ptr               The DAG for which to retrieve the counter output result.
 * @param[in]   cnt_name              The name of the counter.
 * @param[out]  ul_result             The variable to store the counter output result.
 *
 * @return     int                    Error code.
 */
int getCntOutput(DagPtr dag_ptr, const std::string& cnt_name,
                 uint32_t& ul_result);

/**
 * @brief      Retrieve the result of a specified random counter output in the sorting DAG.
 *
 * @param[in]   dag_ptr               The DAG for which to retrieve the random counter output result.
 * @param[in]   cnt_name              The name of the random counter.
 * @param[out]  f_result              Reference to a double variable to store the random counter output result.
 *
 * @return     int                    Error code.
 */
int getCntRandomOutput(DagPtr dag_ptr, const std::string& cnt_name,
                       double& f_result);

/**
 * @brief      Preprocesses data using FFT for controlling the precision of CKKS multiplication.
 *
 * @param[in]   vec_org               The original data.
 * @param[in]   input_name_real       Input name for the real part.
 * @param[in]   input_name_imag       Input name for the imaginary part.
 * @param[out]  inputs                Valuation containing both real and imaginary parts.
 *
 * @return     int                    Error code.
 */
int encodeOrgInputFFT(const std::vector<uint32_t>& vec_org,
                      const std::string& input_name_real,
                      const std::string& input_name_imag, Valuation& inputs);

/**
 * @brief      Retrieve the output results after FFT computation.
 *
 * @param[in]   dag_ptr              The DAG from which to retrieve the FFT results.
 * @param[in]   num_cnt              The total amount of data.
 * @param[in]   output_real_name     The name for the real part of the FFT output.
 * @param[in]   output_imag_name     The name for the imaginary part of the FFT output.
 * @param[out]  vec_results          The vector storing the rounded results that meet the specified conditions.
 *
 * @return     int                   Error code.
 */
int getFFTOutputs(DagPtr dag_ptr, uint32_t num_cnt,
                  const std::string& output_real_name,
                  const std::string& output_imag_name,
                  std::vector<uint32_t>& vec_results);

/**
 * @brief      Retrieve the double-precision output results after FFT computation.
 *
 * @param[in]  dag_ptr                The DAG from which to retrieve the FFT results.
 * @param[in]  num_cnt                The total amount of data.
 * @param[in]  output_real_name       The name for the real part of the FFT output.
 * @param[in]  output_imag_name       The name for the imaginary part of the FFT output.
 * @param[out] vec_results            The vector storing the double-precision output results.
 *
 * @return     int                    Error code.
 */
int getFFTDoubleOutputs(DagPtr dag_ptr, uint32_t num_cnt,
                        const std::string& output_real_name,
                        const std::string& output_imag_name,
                        std::vector<double>& vec_results);

/**
 * @brief      Retrieve the sum of double-precision output results after FFT computation.
 *
 * @param[in]  dag_ptr                The DAG from which to retrieve the FFT results.
 * @param[in]  num_cnt                The total amount of data.
 * @param[in]  output_real_name       The name for the real part of the FFT output.
 * @param[in]  output_imag_name       The name for the imaginary part of the FFT output.
 * @param[out] sum_result             The variable to store the sum of the FFT output results.
 *
 * @return     int                    Error code.
 */
int getFFTSumOutputs(DagPtr dag_ptr, uint32_t num_cnt,
                     const std::string& output_real_name,
                     const std::string& output_imag_name, uint32_t& sum_result);

/**
 * @brief       Check if the computation logic includes bootstrapping.
 *              If bootstrapping is included, separate serialization of bootstrapping keys is required.
 *
 * @param[in]   dag_ptr                   The DAG to check.
 *
 * @return      bool                      True if bootstrapping is included.
 */
bool checkIsBootstrapping(DagPtr dag_ptr);

/**
 * @brief       Serialize by data type, used for handling data exceeding the protobuf size limit.
 *
 * @param[in]   dag_ptr                   The DAG to serialize.
 * @param[in]   type                      The data type, see the SERIALIZE_DATA_TYPE enumeration.
 * @param[out]  stream                    The output stream for serialization.
 *
 * @return      int                       Error code.
 */
int serializeByType(DagPtr dag_ptr, SERIALIZE_DATA_TYPE type,
                    std::ostream& stream);

/**
 * @brief       Deserialize by data type.
 *
 * @param[in]   dag_ptr                  The DAG to deserialize.
 * @param[in]   type                     The data type, see the SERIALIZE_DATA_TYPE enumeration.
 * @param[in]   stream                   The input stream for deserialization.
 *
 * @return      int                      Error code.
 */
int deserializeByType(DagPtr dag_ptr, SERIALIZE_DATA_TYPE type,
                      std::istream& stream);

/**
 * @brief       Interface for constructing the average DAG.
 *
 * @param[in]   group                    The DAG
 * @param[in]   expr                     The expression representing the input for average calculation.
 */
void setAvgGroup(DagPtr group, const Expr expr);

/**
 * @brief      Retrieve the slot size of the DAG.
 *
 * @param[in]  dag_ptr       The DAG for which to retrieve the slot size.
 *
 * @return     uint32_t      The slot size.
 */
uint32_t getVecSize(DagPtr dag_ptr);

/**
 * @brief      Set the actual data size (default: MAX_CMP_NUM).
 *
 * @param[in]  dag_ptr       The DAG for which to set the actual data size.
 * @param[in]  num_cnt       The actual data size to be set.
 */
void setCmpNumSize(DagPtr dag_ptr, uint32_t num_cnt);

/**
 * @brief      Get library information for the DAG.
 *
 * @param[in]  dag_ptr                   The DAG for which to retrieve library information.
 *
 * @return     std::vector<std::string>  A vector of strings containing library information.
 */
std::vector<std::string> getLibInfo(DagPtr dag_ptr);

/**
 * @brief      Get the index of the next node in the DAG.
 *
 * @param[in]  dag_ptr                   The DAG for which to retrieve the next node index.
 *
 * @return     uint64_t                  The index of the next node.
 */
uint64_t getNextNodeIndex(DagPtr dag_ptr);

/**
 * @brief      Set the index of the next node in the DAG.
 *
 * @param[in]  dag_ptr                   The DAG for which to set the next node index.
 * @param[in]  next_node_index           The index to set for the next node.
 */
void setNextNodeIndex(DagPtr dag_ptr, uint64_t next_node_index);

/**
 * @brief      Convert the DAG to a string representation of the graph.
 *
 * @param[in]  dag_ptr                   The DAG to be converted.
 *
 * @return     std::string               A string representation of the graph.
 */
std::string toGraph(DagPtr dag_ptr);

}  // namespace iyfc