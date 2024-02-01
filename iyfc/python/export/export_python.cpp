#include "dag/expr.h"
#include "dag/iyfc_dag.h"
#include "dag/node.h"
#include "dag/data_type.h"
#include "util/math_util.h"
#include "iyfc_include.h"
#include "dag/constant_value.h"
#include "proto/save_load.h"
#include "comm_include.h"
#include "proto/known_type.h"
#include "seal/alo/ckks_parameters.h"

#include <ostream>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>

namespace py = pybind11;

template<typename T> struct Deleter { void operator() (T *o) const { delete o; } };

namespace iyfc {
    Expr SumCntHelperExport(const Expr &lhs) {
        Expr sum_expr = lhs;
        /*
        for (int i = 1; i <= MAX_CMP_NUM - 1; i++) {
            sum_expr += lhs << (i * FFT_N);
        }
        */
        //减少rotation次数
        for (int i = MAX_CMP_NUM / 2; i >=1;) {
            sum_expr += sum_expr << (i * FFT_N);
            i /= 2;
        }

        std::vector<double> vec_mask;
        getSumMaskVec(FFT_N, CMP_DAG_SIZE, vec_mask);
        return sum_expr * vec_mask;
    }

    vector<double> encodeOrgInputforCmpForPython(const std::vector<uint32_t>& vec_org,
                                        const string& input_name) {
        uint32_t num_cnt = vec_org.size();

        vector<double> compose_vec_x;
        if (num_cnt > MAX_CMP_NUM) {
            warn("max cmp  once");
            return compose_vec_x;
        }
        uint32_t p = 3;
        uint32_t composemod = (p - 1) / 2 + 1;

        uint32_t d = 2, l_first = 2, l_second = 2, l_third = 2, l_fourth = 2;
        // The polynomial dimension is currently fixed at 16384

        uint32_t bits = d * l_first * l_second * l_third * l_fourth;
        for (const auto& item : vec_org) {
            vector<uint32_t> tmp_vec = decimalConvert(item, composemod, bits);
            compose_vec_x.insert(compose_vec_x.end(), tmp_vec.begin(), tmp_vec.end());
        }
        compose_vec_x.resize(CMP_DAG_SIZE);

       return compose_vec_x;
    }

    std::tuple<std::vector<double>, std::vector<double>>  encodeOrgInputFFTForPython(const std::vector<uint32_t>& vec_org) {
        std::vector<double> vec_real;
        std::vector<double> vec_imag;

        FastFourierTransform fft_helper(FFT_N, FFTW_FORWARD);

        for (const auto& item : vec_org) {
            std::vector<int> vec_num;
            getNumReVec(item, vec_num);
            for (int i = 0; i < FFT_N; i++) {
            fft_helper.m_in[i][0] = double(vec_num[i]);
            fft_helper.m_in[i][1] = 0;
            }
            // fft
            fft_helper.fft();
            for (int i = 0; i < FFT_N; i++) {
            vec_real.push_back(fft_helper.m_out[i][0]);
            vec_imag.push_back(fft_helper.m_out[i][1]);
            }
        }

        vec_real.resize(CMP_DAG_SIZE);
        vec_imag.resize(CMP_DAG_SIZE);

        
        return std::make_tuple(vec_real, vec_imag);
    }

    // std::unique_ptr<iyfc::Dag> loadFromFileForPython(const std::string &path) {
    //     return std::get<std::unique_ptr<iyfc::Dag>>(iyfc::loadFromFile(path));
    // }

    template <class T>
        T loadFromFileForPython(const std::string &path) {
        return std::get<T>(loadFromFile(path));
    }

    std::vector<uint32_t> getFFTOutputsForPython(DagPtr dag_ptr, uint32_t num_cnt,
                                 const string& output_real_name,
                                 const string& output_imag_name) {
        std::vector<uint32_t> vec_results;
        Valuation outputs;
        dag_ptr->getDecryptOutput(outputs);

        // Real part processing
        std::vector<double> vec_real;
        if (outputs.find(output_real_name) != outputs.end()) {
            vec_real = std::get<std::vector<double>>(outputs[output_real_name]);
        } else {
            warn("err real outputs");
            return vec_results;
        }

        std::vector<double> vec_imag;
        if (outputs.find(output_imag_name) != outputs.end()) {
            vec_imag = std::get<std::vector<double>>(outputs[output_imag_name]);
        } else {
            warn("err real outputs");
            return vec_results;
        }
        int total_cnt = num_cnt * FFT_N;
        if (vec_real.size() < total_cnt || vec_imag.size() < total_cnt) {
            warn("err complex outputs size");
            return vec_results;
        }

        vec_real.resize(total_cnt);
        vec_imag.resize(total_cnt);

        // 32 bits represent one number
        FastFourierTransform fft_helper(FFT_N, FFTW_BACKWARD);
        int cur_index = 0;
        for (int i = 0; i < total_cnt; i++) {
            fft_helper.m_in[cur_index][0] = vec_real[i];
            fft_helper.m_in[cur_index][1] = vec_imag[i];
            cur_index++;

            if (cur_index == FFT_N) {
            fft_helper.fft();
            vec_results.emplace_back(getComplexNum(fft_helper.m_out, FFT_N));
            cur_index = 0;
            }
        }

        return vec_results;
    }
}

// Export the basic classes of iyfc to python
PYBIND11_MODULE(pyiyfc, m) {
    m.doc() = "Python APIs for iyfc"; 

    m.def("QuerySum", &iyfc::SumCntHelperExport);
    m.def("encodeOrgInputforCmp", &iyfc::encodeOrgInputforCmp);
    m.def("encodeOrgInputFFT", &iyfc::encodeOrgInputFFT);
    m.def("getFFTOutputs", &iyfc::getFFTOutputs);
    m.def("encodeOrgInputforCmpForPython", &iyfc::encodeOrgInputforCmpForPython);
    m.def("encodeOrgInputFFTForPython", &iyfc::encodeOrgInputFFTForPython);
    m.def("getFFTOutputsForPython", &iyfc::getFFTOutputsForPython);

    m.def("saveToFile", &iyfc::saveToFile<iyfc::Dag>);
    // m.def("loadFromFile", &iyfc::loadFromFileForPython<std::unique_ptr<iyfc::CKKSParameters>>, py::return_value_policy::reference, py::keep_alive<0, 1>());
    m.def("loadFromFile", &iyfc::loadFromFileForPython<std::unique_ptr<iyfc::Dag>>, py::return_value_policy::take_ownership, py::keep_alive<0, 1>());


    // py::class_<iyfc::ContantValue<std::int64_t>>(m, "ContantValue")
    //     .def(py::init<std::size_t>())
    //     .def_readwrite("expand", &iyfc::ContantValue::expand, py::return_value_policy::reference)
    //     .def_readwrite("expandTo", &iyfc::ContantValue::expandTo)
    //     .def_readwrite("isZero", &iyfc::ContantValue::isZero)
    //     .def_readwrite("serialize", &iyfc::ContantValue::serialize);


    py::class_<iyfc::CKKSParameters>(m, "CKKSParameters")
        .def(py::init<>())
        .def_readwrite("prime_bits", &iyfc::CKKSParameters::prime_bits)
        .def_readwrite("rotations", &iyfc::CKKSParameters::rotations)
        .def_readwrite("poly_modulus_degree", &iyfc::CKKSParameters::poly_modulus_degree);

    py::class_<iyfc::DagSerializePara>(m, "DagSerializePara")
        .def(py::init<bool, bool, bool, bool, bool, bool>())
        .def_readwrite("need_node_info", &iyfc::DagSerializePara::need_node_info)
        .def_readwrite("need_genkey_info", &iyfc::DagSerializePara::need_genkey_info)
        .def_readwrite("need_sig_info", &iyfc::DagSerializePara::need_sig_info)
        .def_readwrite("need_exe_ctx", &iyfc::DagSerializePara::need_exe_ctx)
        .def_readwrite("need_encrpt_ctx", &iyfc::DagSerializePara::need_encrpt_ctx)
        .def_readwrite("need_decrypt_ctx", &iyfc::DagSerializePara::need_decrypt_ctx);

// TODO: Two methods with unique_ptr inputs
    py::class_<iyfc::Dag, std::shared_ptr<iyfc::Dag>>(m, "Dag")
        .def(py::init<std::string, std::uint64_t>())
        .def("setInput", &iyfc::Dag::setInput)
        .def("setOutput", &iyfc::Dag::setOutput)
        .def("makeNode", &iyfc::Dag::makeNode, py::return_value_policy::reference)
        .def("makeUint32Const", &iyfc::Dag::makeUint32Const, py::return_value_policy::reference)
        .def("makeDenseConstant", &iyfc::Dag::makeDenseConstant, py::return_value_policy::reference)
        .def("makeInt64DenseConstant", &iyfc::Dag::makeInt64DenseConstant, py::return_value_policy::reference)
        .def("makeUniformConstant", &iyfc::Dag::makeUniformConstant, py::return_value_policy::reference)
        .def("makeInput", &iyfc::Dag::makeInput, py::return_value_policy::reference)
        .def("makeOutput", &iyfc::Dag::makeOutput, py::return_value_policy::reference)
        .def("makeLeftRotation", &iyfc::Dag::makeLeftRotation, py::return_value_policy::reference)
        .def("makeRightRotation", &iyfc::Dag::makeRightRotation, py::return_value_policy::reference)
        .def("makeRescale", &iyfc::Dag::makeRescale, py::return_value_policy::reference)
        .def("getInput", &iyfc::Dag::getInput, py::return_value_policy::reference)
        .def("supportShortInt", &iyfc::Dag::supportShortInt)
        .def("setSupportShortInt", &iyfc::Dag::setSupportShortInt)
        .def("collectExprNode", &iyfc::Dag::collectExprNode)
        .def("freeNode", &iyfc::Dag::freeNode)
        .def("setSecLevel", &iyfc::Dag::setSecLevel)
        .def("setMulticore", &iyfc::Dag::setMulticore)
        .def("getInputs", &iyfc::Dag::getInputs, py::return_value_policy::reference)
        .def("getOutputs", &iyfc::Dag::getOutputs, py::return_value_policy::reference)
        .def("getName", &iyfc::Dag::getName)
        .def("setName", &iyfc::Dag::setName)
        .def("getVecSize", &iyfc::Dag::getVecSize)
        .def("setVecSize", &iyfc::Dag::setVecSize)
        .def("getSources", &iyfc::Dag::getSources, py::return_value_policy::reference)
        .def("getSinks", &iyfc::Dag::getSinks, py::return_value_policy::reference)
        .def("deepCopy", &iyfc::Dag::deepCopy, py::return_value_policy::reference)
        .def("toDOT", &iyfc::Dag::toDOT)
        .def("printNodeCnt", &iyfc::Dag::printNodeCnt)
        .def("doTranspile", &iyfc::Dag::doTranspile)
        .def("genKey", &iyfc::Dag::genKey)
        .def("encryptInput", &iyfc::Dag::encryptInput)
        .def("executor", &iyfc::Dag::executor)
        .def("getDecryptOutput", &iyfc::Dag::getDecryptOutput)
        .def("getDecryptOutputForPython", &iyfc::Dag::getDecryptOutputForPython)
        .def("saveAloInfoToFile", &iyfc::Dag::saveAloInfoToFile)
        .def("loadAloInfoFromFile", &iyfc::Dag::loadAloInfoFromFile)
        .def("saveInputToFile", &iyfc::Dag::saveInputToFile)
        .def("loadInputFromFile", &iyfc::Dag::loadInputFromFile)
        .def("saveOutputToFile", &iyfc::Dag::saveOutputToFile)
        .def("loadOutputFromFile", &iyfc::Dag::loadOutputFromFile)
        .def("saveAloInfoToStr", &iyfc::Dag::saveAloInfoToStr)
        .def("loadAloInfoFromStr", &iyfc::Dag::loadAloInfoFromStr)
        .def("saveInputToStr", &iyfc::Dag::saveInputToStr)
        .def("loadInputFromStr", &iyfc::Dag::loadInputFromStr)
        .def("saveOutputToStr", &iyfc::Dag::saveOutputToStr)
        .def("loadOutputFromStr", &iyfc::Dag::loadOutputFromStr)
        .def("eraseSource", &iyfc::Dag::eraseSource)
        .def("configInputScale", &iyfc::Dag::configInputScale)
        .def("configOutputRange", &iyfc::Dag::configOutputRange)
        .def_readwrite("m_short_int", &iyfc::Dag::m_short_int)
        .def_readwrite("m_has_int64", &iyfc::Dag::m_has_int64)
        .def_readwrite("m_has_double", &iyfc::Dag::m_has_double);
        // .def_write_only("m_serialize_para", &iyfc::Dag::m_serialize_para);
        // .def("makeInt64Constant", &iyfc::Dag::makeInt64Constant);
        // .def("makeConstant", &iyfc::Dag::makeConstant, py::return_value_policy::reference);

// TODO：Improve operator
    py::class_<iyfc::Expr>(m, "Expr")
        .def(py::init<iyfc::Dag*, iyfc::NodePtr>())
        .def(py::init<iyfc::Dag*, double>())
        .def(py::init<iyfc::Dag*, int64_t>())
        .def(py::init<iyfc::Dag*, uint32_t>())
        .def(py::init<iyfc::Dag*, const std::vector<double>&>())
        .def(py::init<iyfc::Dag*, const std::vector<int64_t>&>())
        .def("negate", &iyfc::Expr::operator!)
        .def(py::self += py::self) // Addition and self assignment
        .def(py::self += std::vector<double>())
        .def(py::self += std::vector<int64_t>())
        .def(py::self += double())
        .def(py::self += int64_t())
        .def(py::self + py::self) // add
        .def(py::self + std::vector<double>())
        .def(std::vector<double>() + py::self)
        .def(py::self + std::vector<int64_t>())
        .def(std::vector<int64_t>() + py::self)
        .def(py::self + double())
        .def(double() + py::self)
        .def(py::self + int64_t())
        .def(int64_t() + py::self)
        .def(py::self - py::self) //subtraction
        .def(py::self - std::vector<double>())
        .def(std::vector<double>() - py::self)
        .def(py::self - std::vector<int64_t>())
        .def(std::vector<int64_t>() - py::self)
        .def(py::self - double())
        .def(double() - py::self)
        .def(py::self - int64_t())
        .def(int64_t() - py::self)
        .def(py::self * py::self) // multiplication
        .def(py::self * std::vector<double>())
        .def(std::vector<double>() * py::self)
        .def(py::self * std::vector<int64_t>())
        .def(std::vector<int64_t>() * py::self)
        .def(py::self * double())
        .def(double() * py::self)
        .def(py::self * int64_t())
        .def(int64_t() * py::self)
        .def(py::self / py::self) // division, TODO:complex division
        .def(std::uint32_t() / py::self) 
        .def(py::self / std::uint32_t())
        .def(py::self -= std::int64_t()) //subtraction and self assignment, TODO: Expr
        .def(py::self -= double())
        .def(py::self -= std::vector<int64_t>())
        .def(py::self -= std::vector<double>())
        .def(py::self *= std::int64_t()) //multiplication and self assignment, TODO: vector/Expr
        .def(py::self *= double())
        .def(py::self != py::self) // not equal
        .def(py::self >= py::self) // greater or equal
        .def(py::self <= py::self) // smaller or equal
        .def(py::self < py::self) // smaller
        .def(py::self > py::self) // greater
        .def(py::self == py::self); // equal

    py::enum_<iyfc::OpType>(m, "OpType")
        .value("Undef", iyfc::OpType::Undef)
        .value("Input", iyfc::OpType::Input)
        .value("Output", iyfc::OpType::Output)
        .value("Constant", iyfc::OpType::Constant)
        .value("U32Constant", iyfc::OpType::U32Constant)
        .value("Negate", iyfc::OpType::Negate)
        .value("Add", iyfc::OpType::Add)
        .value("Sub", iyfc::OpType::Sub)
        .value("Mul", iyfc::OpType::Mul)
        .value("Div", iyfc::OpType::Div)
        .value("Equality", iyfc::OpType::Equality)
        .value("Greater", iyfc::OpType::Greater)
        .value("Smaller", iyfc::OpType::Smaller)
        .value("RotateLeftConst", iyfc::OpType::RotateLeftConst)
        .value("RotateRightConst", iyfc::OpType::RotateRightConst)
        .value("Relinearize", iyfc::OpType::Relinearize)
        .value("ModSwitch", iyfc::OpType::ModSwitch)
        .value("Rescale", iyfc::OpType::Rescale)
        .value("Encode", iyfc::OpType::Encode);

    py::enum_<iyfc::DataType>(m, "DataType")
        .value("Undef", iyfc::DataType::Undef)
        .value("Cipher", iyfc::DataType::Cipher)
        .value("Raw", iyfc::DataType::Raw)
        .value("Plain", iyfc::DataType::Plain);

// TODO：Clarify the template parameters of has, get, and set in AttrList
    py::class_<iyfc::AttrList, std::unique_ptr<iyfc::AttrList, Deleter<iyfc::AttrList>>>(m, "AttrList")
        .def(py::init<>())
        .def("loadAttr", &iyfc::AttrList::loadAttr)
        .def("serializeAttr", &iyfc::AttrList::serializeAttr)
        .def("assignAttrFrom", &iyfc::AttrList::assignAttrFrom);

    py::class_<iyfc::Node, std::shared_ptr<iyfc::Node>>(m, "Node")
        .def(py::init<iyfc::OpType, iyfc::Dag*>())
        .def("addOperand", &iyfc::Node::addOperand)
        .def("eraseOperand", &iyfc::Node::eraseOperand)
        .def("eraseAllOperand", &iyfc::Node::eraseAllOperand)
        .def("replaceOperand", &iyfc::Node::replaceOperand)
        .def("setOperands", &iyfc::Node::setOperands)
        .def("numOperands", &iyfc::Node::numOperands)
        .def("operandAt", &iyfc::Node::operandAt)
        .def("getOperands", &iyfc::Node::getOperands, py::return_value_policy::reference)
        .def("replaceUsesWithIf", &iyfc::Node::replaceUsesWithIf)
        .def("replaceAllUsesWith", &iyfc::Node::replaceAllUsesWith)
        .def("replaceOtherUsesWith", &iyfc::Node::replaceOtherUsesWith)
        .def("numUses", &iyfc::Node::numUses)
        .def("getUses", &iyfc::Node::getUses)
        .def("isInternal", &iyfc::Node::isInternal)
        .def_readonly("m_op_type", &iyfc::Node::m_op_type)
        .def_readonly("m_index", &iyfc::Node::m_index)
        .def_readwrite("m_use", &iyfc::Node::m_use)
        .def_readwrite("m_dag", &iyfc::Node::m_dag)
        .def("__repr__", [](const iyfc::Node &node) {
            return "<Node " + std::to_string(node.m_index) + ">";
        });

}
