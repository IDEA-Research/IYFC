import pyiyfc as iyfc


def test_math(cal_type='Add'):
    print("------------------test_math: {0}------------------".format(cal_type))
    dag = iyfc.Dag("test_python", 16)

    lhs = dag.setInput("lhs", iyfc.DataType.Raw)
    lhs_2 = dag.setInput("lhs_2", iyfc.DataType.Raw)

    rhs = dag.setInput("rhs", iyfc.DataType.Raw)
    rhs_2 = dag.setInput("rhs_2", iyfc.DataType.Raw)

    if cal_type == 'Add':
        dag.setOutput("output", rhs + lhs)
        dag.setOutput("output_2", lhs_2 + rhs_2)
    elif cal_type == 'Sub':
        dag.setOutput("output", rhs - lhs)
        dag.setOutput("output_2", lhs_2 - rhs_2)
    elif cal_type == 'Mul':
        dag.setOutput("output", rhs * lhs)
        dag.setOutput("output_2", lhs_2 * rhs_2)
    elif cal_type == 'Div':
        dag.setOutput("output", rhs / lhs)
        dag.setOutput("output_2", lhs_2 / rhs_2)
    else:
        raise Exception("cal_type error")

    dag.doTranspile()
    dag.genKey()

    inputs = {"lhs": 6, "rhs": 8, "lhs_2": 3, "rhs_2": 4}
    dag.encryptInput(inputs)
    dag.executor()

    result = dag.getDecryptOutputForPython()
    for k, v in result.items():
        print(k, v)


if __name__ == '__main__':
    test_math('Add')
    test_math('Sub')
    test_math('Mul')
    test_math('Div')
