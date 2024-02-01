
#  🤖 iyfc-fhe-translator ⌨️
IYFC是一个针对同态加密算法库转译器，覆盖主流同态加密算法库 [Microsoft SEAL](https://github.com/microsoft/SEAL), [OpenFHE](https://github.com/openfheorg/openfhe-development),  [Concrete](https://github.com/zama-ai/concrete)。
IYFC引入了一个用户友好的接口，为密码学专家和非专家提供更易访问的体验。此外，IYFC集成智能决策模块，自动评估用户需求，推荐在特定场景下最合适的同态加密库，并考虑性能、安全性和易用性等因素。
更多的，IYFC通过多项式模拟比较查询算子等逻辑算子，为用户提供便捷比较查询能力。
特别的,IYFC子项目使用cuda 重写加速seal库，加速同态加密流程，SEAL-GPU子项目开源计划中。
通过简化实现、库选择和参数配置流程，IYFC赋予用户利用同态加密能力的能力，而不受现有库所带来的复杂性的阻碍。我们的框架是向实际应用中更广泛采用同态加密迈出的一步，促进了更安全和隐私保护的计算环境。
如果想快速开始 见[快速开始](# 快速开始)
如果想详细了解项目依赖 见[详细安装步骤](# 详细安装步骤)

## 快速开始 

### 基础编译环境Requirements
Linux	Clang++ (>= 5.0) or GNU G++ (>= 6.0), CMake (>= 3.13)

## CPU环境 快速开始

首次使用,需要sudo命令权限,一键安装依赖（具体依赖说明见详细安装步骤）

mkdir build
cd build
cmake .. -DIYFC_BUILD_DEPS=ON -DAUTO_BUILD_FFT=ON -DAUTO_INSTALL_CARGO=ON -DAUTO_INSTALL_OPENMP=ON -DIYFC_BUILD_TESTS=OFF
make 
make install

### 使用时CMakeLists 
```shell
find_package(IYFC REQUIRED)
target_link_libraries(your_test IYFC::iyfc )
```

## GPU环境 快速开始
GPU环境建议直接使用项目提供的dockerfile
GPU_SEAL代码开源,计划中

## 详细安装步骤

### 步骤1：依赖protobuf
建议 protobuf > 3.2
sudo apt install cmake libboost-all-dev libprotobuf-dev protobuf-compiler

### 步骤2：依赖seal库
sudo apt install clang
sudo update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100
git clone -b v4.0.0 https://github.com/microsoft/SEAL.git
cd SEAL
cmake -DSEAL_THROW_ON_TRANSPARENT_CIPHERTEXT=OFF .
make -j
sudo make install

### 步骤3：concrete库

cargo 安装
snap install rustup  # version 1.24.3, or
apt  install cargo   # version 0.62.0ubuntu0libgit2-0ubuntu0.20.04.1
yum install cargo    # centos7
sudo apt install build-essential
sudo apt-get update && sudo apt-get install -y libfftw3-dev libssl-dev

### 步骤4：openfhe库安装
https://openfhe-development.readthedocs.io/en/latest/sphinx_rsts/intro/installation/linux.html
openfhe 默认是安装so  建议修改CmakeList install静态库 

### 步骤5：FFT安装
sudo apt-get install libfftw3-dev

### 步骤6编译
Ubuntu
cmake -S . -B build


### 🪜🪜 编码步骤

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

## 💐 致谢
我们的项目与[Microsoft SEAL](https://github.com/microsoft/SEAL), [OpenFHE](https://github.com/openfheorg/openfhe-development), and [Concrete](https://github.com/zama-ai/concrete), [EVA](https://github.com/microsoft/EVA/tree/main/eva),等相关,SEAL OPENFHE CONCRETE为我们提供了同态加密标准方案，在针对SEAL库ckks方案的优化上，参照了EVA项目的部分策略，没有他们的工作我们无法进行此项目，感谢他们的伟大工作


## 计划事项
- [ ] seal-gpu库开源计划