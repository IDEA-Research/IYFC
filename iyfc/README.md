#  🤖 iyfc-fhe-translator ⌨️

IYFC is a transpiler designed for homomorphic encryption algorithm libraries, covering mainstream libraries such as [Microsoft SEAL](https://github.com/microsoft/SEAL), [OpenFHE](https://github.com/openfheorg/openfhe-development), and [Concrete](https://github.com/zama-ai/concrete). 
IYFC introduces a user-friendly interface, providing a more accessible experience for both cryptography experts and non-experts. Additionally, IYFC integrates an intelligent decision-making module that automatically assesses user requirements, recommends the most suitable homomorphic encryption library for a given scenario, and considers factors such as performance, security, and usability.

Furthermore, IYFC facilitates convenient comparative query capabilities for users through the implementation of logical operators like polynomial-simulated comparison query operators.

Particularly, a sub-project of IYFC rewrites the SEAL library using CUDA to accelerate the homomorphic encryption process, with plans for the open-source release of the SEAL-GPU sub-project.

By simplifying the implementation, library selection, and parameter configuration processes, IYFC empowers users to harness the capabilities of homomorphic encryption without being hindered by the complexities associated with existing libraries.

Our framework represents a significant step toward the broader adoption of homomorphic encryption in real-world applications, fostering a more secure and privacy-preserving computational environment.

For a quick start, please refer to [Quick Start](# Quick Start).

For a detailed understanding of project dependencies, please refer to [Detailed Installation Steps](# Detailed Installation Steps).

## 🔥 News

- **`2024/2/2`**: first commit~

## Quick Start

### Basic Compilation Environment Requirements

Linux Clang++ (>= 5.0) or GNU G++ (>= 6.0), CMake (>= 3.13)

### Quick Start in CPU Environment

For the initial setup, sudo command privileges are required. The one-click installation of dependencies is available (refer to the detailed installation steps for specific dependency information).

```shell
mkdir build
cd build
cmake .. -DIYFC_BUILD_DEPS=ON -DAUTO_BUILD_FFT=ON -DAUTO_INSTALL_CARGO=ON -DAUTO_INSTALL_OPENMP=ON -DIYFC_BUILD_TESTS=OFF
make 
make install
```

### CMakeLists for User

```shell
find_package(IYFC REQUIRED)
target_link_libraries(your_test IYFC::iyfc )
```

### Quick Start in GPU Environment

For GPU environments, it is recommended to use the provided Dockerfile directly. 
The GPU_SEAL code is open source, with plans underway.


## 🪄 Detailed Installation Steps

### 1. protobuf

Recommended protobuf version is greater than 3.2.

```shell
sudo apt install cmake libboost-all-dev libprotobuf-dev protobuf-translator
```

### 2. SEAL

- #### Install Clang translator

```shell
sudo apt install clang
sudo update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100
```

- #### Clone and Build SEAL Library

```shell
git clone -b v4.0.0 https://github.com/microsoft/SEAL.git
cd SEAL
cmake -DSEAL_THROW_ON_TRANSPARENT_CIPHERTEXT=OFF .
make -j
sudo make install
```

### 3. Rust

The [concrete](./concrete) subdirectory contains code related to interface bindings between C/C++ and Rust. These interfaces enable C/C++ code to call Rust functions, which perform various encryption and mathematical operations, including encryption, decryption, addition, subtraction, multiplication, rotation, and more.

- #### Install Cargo

Using Snap (version 1.24.3)

```shell
snap install rustup  # version 1.24.3, or
```

Using APT (version 0.62.0)

```shell
apt  install cargo   # version 0.62.0ubuntu0libgit2-0ubuntu0.20.04.1
```

Using YUM (CentOS 7)

```shell
yum install cargo #centos7
```

- #### Install Build Essential and Dependencies

```shell
sudo apt install build-essential
sudo apt-get update && sudo apt-get install -y libfftw3-dev libssl-dev
```

### 4. Openfhe

For detailed installation instructions, please refer to the official documentation:

[Installing OpenFHE on Linux](https://openfhe-development.readthedocs.io/en/latest/sphinx_rsts/intro/installation/linux.html)

By default, openfhe is installed as a shared library. To debug or make modifications, you can install it as a static library with the following commands:

```shell
git clone https://github.com/openfheorg/openfhe-development.git
cmake -DEMSCRIPTEN=ON ..
```

### 5. FFT

```shell
sudo apt-get install libfftw3-dev
```

### 6. Building and Compiling

On Ubuntu:

```shell
cmake -S . -B build
```


## 🔖 Tips for fhe-translator

You can find [examples ](./examples)  to quickly get started with fhe-translator.🥳🥳🥳

Next, I will briefly introduce the main concepts and steps involved in fhe-translator. Hopefully, you will find this helpful.

### 📑📑 Main Concepts

- **DAG**: The overall computational logic is represented as a directed acyclic graph (DAG), which contains information about nodes, algorithm used, encryption keys and so on.

* **Expr**: Means expressions.  Overloaded C++  operators in class expr in order to construct the DAG.
* **Input**: Input nodes in the DAG graph, supporting multiple inputs.
* **Output**: Output nodes in the DAG graph, requiring association with expressions.
* **Serialization**: Supports portability of DAG, input and output. Depending on the parameter settings by `setDagSerializePara()`, different content in the DAG can be ported (e.g., nodes, public key environments, private key environments, signatures).

### 🪜🪜 Steps

* step1  Initialize a new Dag with Dag name and vec size (1024 by default)

```cpp
DagPtr dag = initDag("hello world");
```

* step2  Set input names

```cpp
Expr input_expr = setInputName(dag, "x");
```

* step3  Define calculation logic

e.g 1.,:

**Real number vector**: **(+ - *)** Support addition, subtraction, and multiplication

**Integer**: **(+*-)** Support addition, subtraction, and multiplication

📢Internal operations are converted to double for real number computations.

```cpp
Expr tmp_expr_1 = Expr(dag, {1.0}); // Currently supports real number vectors
Expr tmp_expr_2 = Expr(dag, {1.0, 2.0});
Expr tmp_expr_out = tmp_expr_1 * tmp_expr_2;
setOutput(dag, "z", tmp_expr_out);
// setOutput(dag, "z", tmp_expr_out * tmp_expr_2);
```

e.g 2.,:

📢Currently, uint3 integer is not practical and is reserved for future use. 

**Uint3 integer**: **(+ - * /)** Support addition, subtraction, multiplication and division.

```cpp
Expr tmp_expr_1 = Expr(dag, 6);
setOutput(dag,"z", tmp_expr_1 / 2);
```

* step4  Set output nodes


```cpp
setOutput(dag, "z" , tmp_expr_out);
```

* step5  Serialize the DAG —— choose different serialization solutions based on different scenarios
* step6  Compile the DAG

```cpp
compileDag(dag);
```

* step7  Generate public and private key

```cpp
genKeys(dag);
```

* step8  Encrypt inputs.

```cpp
Valuation inputs{{"x", vec_input}};
encryptInput(dag, inputs);
```

* step9  Execute the DAG

```cpp
exeDag(dag);
```

* step10  Decrypt the outputs

```cpp
Valuation outputs;
decryptOutput(dag, outputs);
```

* step11  Release resources

```cpp
releaseDag(dag);
```

## 📌 TODO

- [ ] Open Source Plan for SEAL-GPU Library

## 💐 Acknowledgment

Our project is related to [Microsoft SEAL](https://github.com/microsoft/SEAL), [OpenFHE](https://github.com/openfheorg/openfhe-development), and [Concrete](https://github.com/zama-ai/concrete), [EVA](https://github.com/microsoft/EVA/tree/main/eva), and other similar initiatives. SEAL, OPENFHE, and CONCRETE have provided us with standard homomorphic encryption schemes. In optimizing for the CKKS scheme within the SEAL library, we have referred to certain strategies from the EVA project. Without their contributions, our project would not have been possible. We express our gratitude for their tremendous work.

## 📎 Citation
