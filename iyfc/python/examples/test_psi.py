import pyiyfc as iyfc 

import random

def test_psi():
    print("------------------test_psi------------------")
    dag = iyfc.Dag("test_python", 1024)

    lhs = dag.setInput("lhs", iyfc.DataType.Raw)



    # Use random.normalvariate to generate a random number sequence with a mean of 10, a variance of 0.1, and a length of 6
    output_random = [random.normalvariate(8, 0.1) for _ in range(8)]


    dag.setOutput("output", output_random - lhs)

    dag.doTranspile()
    dag.genKey()


    # The requirements here are strict on whether the input is int or float. Later, we will try to adapt automatically on the python side.
    inputs = {"lhs": [8.0 for _ in range(1024)]}
    dag.encryptInput(inputs)
    dag.executor()

    result = dag.getDecryptOutputForPython()
    for k, v in result.items():
        print(k, v)


if __name__ == '__main__':
    test_psi()