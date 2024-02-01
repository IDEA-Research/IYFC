
import pyiyfc as iyfc

def test_query():
    print("------------------test_query------------------")
    dag = iyfc.Dag("test_python", 4096)

    lhs = dag.setInput("lhs", iyfc.DataType.Cipher)
    rhs = dag.setInput("rhs", iyfc.DataType.Cipher)

    lhs_2 = dag.setInput("lhs_2", iyfc.DataType.Cipher)
    rhs_2 = dag.setInput("rhs_2", iyfc.DataType.Cipher)

    fft_real = dag.setInput("fft_real", iyfc.DataType.Cipher)
    fft_imag = dag.setInput("fft_imag", iyfc.DataType.Cipher)

    dag.setOutput("output_real", iyfc.QuerySum(((lhs <= rhs) * (lhs_2 != rhs_2)) * fft_real))
    dag.setOutput("output_imag", iyfc.QuerySum(((lhs <= rhs) * (lhs_2 != rhs_2)) * fft_imag))

    dag.doTranspile()
    dag.genKey()
    
    inputs = {"lhs": [1, 2, 5, 100, 1000], 
              "rhs": [2, 2, 4, 100, 1001], 
              "lhs_2": [4, 7, 9, 5, 100], 
              "rhs_2": [4, 3, 5, 100, 100], 
              "fft_real": [], 
              "fft_imag": []}
    
    org_vec = [200, 22, 2, 100, 1]

    inputs["lhs"] = iyfc.encodeOrgInputforCmpForPython(inputs['lhs'], "lhs")
    inputs["rhs"] = iyfc.encodeOrgInputforCmpForPython(inputs['rhs'], "rhs")
    inputs["lhs_2"] = iyfc.encodeOrgInputforCmpForPython(inputs['lhs_2'], "lhs_2")
    inputs["rhs_2"] = iyfc.encodeOrgInputforCmpForPython(inputs['rhs_2'], "rhs_2")
    inputs["fft_real"], inputs["fft_imag"] = iyfc.encodeOrgInputFFTForPython(org_vec)

    for k, v in inputs.items():
        print(k, len(v))


    dag.encryptInput(inputs)
    dag.executor()

    result = iyfc.getFFTOutputsForPython(dag, len(org_vec), "output_real", "output_imag")
    print(result)


if __name__ == '__main__':
    test_query()