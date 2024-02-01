import pyiyfc as iyfc 
import inspect
import random


# Build a decorator using hfc to encrypt the calculation logic
def hfc_math_test(name='encrypt', 
                enable_save=False,
                dag_size=1024):
    def build(func):
        def wrapper(*args, **kwargs):
            print("=============iyfc加密及测试=============")
            def generate_cal_logic(cal_func):
                source_lines, _ = inspect.getsourcelines(cal_func)
                source_code = ''.join(source_lines)

                st = "return "
                start_index = source_code.find('return ') + len(st)
                end_index = source_code.find('\n', start_index)
                logic = source_code[start_index:end_index].strip()
                multi_logic = logic.split(',')

                return multi_logic

            # Generate and print the calculation logic
            logic = generate_cal_logic(func)
            print(logic)
            inputs = func.__code__.co_varnames            
            # Construct dictionaries for original and encrypted expressions
            var_dic = {}
            for input in inputs:
                var_dic[input] = "input_expr[{}]".format(inputs.index(input))
            for key, value in var_dic.items():
                for i in range(len(logic)):
                    logic[i] = logic[i].replace(key, value)
                # logic = logic.replace(key, value)
            print(logic)
            
             # Construct inputs for the computation DAG
            file_name = name.split('.')[0]
            dag = iyfc.Dag(file_name, dag_size)
            input_expr = []
            for input in inputs:
                input_expr.append(dag.setInput(input, iyfc.DataType.Raw))

            # Multiple outputs
            for i in range(len(logic)):
                dag.setOutput("output_" + str(i), eval(logic[i]))

             # Compile the computation DAG
            dag.doTranspile()
            dag.genKey()

            # Generate random encrypted input for testing
            test_input = {}
            for input in inputs:
                test_input[input] = random.randint(1, 5)
            print("test_input: ", test_input)
            dag.encryptInput(test_input)
            dag.executor()

            if enable_save:
                iyfc.saveToFile(dag, name + '/dag.txt')
                dag.saveAloInfoToFile(name + '/alo.txt')
                dag.saveInputToFile(name + '/input.txt')
                dag.saveOutputToFile(name + '/output.txt')
                print("=======Files saved successfully=======")

            result = dag.getDecryptOutputForPython()
            print(result)
            print("=============iyfc Encryption and Testing completed, following is the normal logic execution=============")
            return func(*args, **kwargs)
        return wrapper
    return build


def hfc_query_test(name='encrypt', 
                enable_save=False,
                dag_size=1024):
    def build(func):
        def wrapper(*args, **kwargs):
            print("=============iyfc Encrypted Query Test=============")
            def generate_cal_logic(cal_func):
                source_lines, _ = inspect.getsourcelines(cal_func)
                source_code = ''.join(source_lines)

                st = "return "
                start_index = source_code.find('return ') + len(st)
                end_index = source_code.find('\n', start_index)
                logic = source_code[start_index:end_index].strip()

                return "(" + logic + ")"

            # Generate and print the query logic
            logic = generate_cal_logic(func)
            print(logic)
            inputs = func.__code__.co_varnames            
            # Construct dictionaries for original and encrypted expressions
            var_dic = {}
            for input in inputs:
                var_dic[input] = "input_expr[{}]".format(inputs.index(input))
            for key, value in var_dic.items():
                logic = logic.replace(key, value)
            print(logic)
            
            # Construct inputs for the computation DAG
            file_name = name.split('.')[0]
            dag = iyfc.Dag(file_name, dag_size)
            fft_real = dag.setInput("fft_real", iyfc.DataType.Cipher)
            fft_imag = dag.setInput("fft_imag", iyfc.DataType.Cipher)
            input_expr = []
            for input in inputs:
                input_expr.append(dag.setInput(input, iyfc.DataType.Cipher))

            # Outputs
            dag.setOutput("output_real", eval("iyfc.QuerySum(" + logic + " * fft_real)"))
            dag.setOutput("output_imag", eval("iyfc.QuerySum(" + logic + " * fft_imag)"))

            # Compile the computation DAG
            dag.doTranspile()
            dag.genKey()

            # Generate random encrypted input for testing
            test_input = {}
            for input in inputs:
                test_input[input] = [random.randint(1, 10) for _ in range(5)]
            print("original_input: ", test_input)

            for input in inputs:
                test_input[input] = iyfc.encodeOrgInputforCmpForPython(test_input[input], input)
            
            org_vec = [200, 22, 2, 100, 1]
            print("org_vec: ", org_vec)
            test_input["fft_real"], test_input["fft_imag"] = iyfc.encodeOrgInputFFTForPython(org_vec)
            

            dag.encryptInput(test_input)
            dag.executor()

            if enable_save:
                iyfc.saveToFile(dag, name + '/dag.txt')
                dag.saveAloInfoToFile(name + '/alo.txt')
                dag.saveInputToFile(name + '/input.txt')
                dag.saveOutputToFile(name + '/output.txt')
                print("=======Files saved successfully=======")

            result = iyfc.getFFTOutputsForPython(dag, len(org_vec), "output_real", "output_imag")
            print(result)
            print("=============iyfc Encrypted Query Test completed=============")
            return func(*args, **kwargs)
        return wrapper
    return build
