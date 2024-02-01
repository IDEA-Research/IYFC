cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-fPIC)
# Set to expose only the interfaces defined in iyfc
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
project(IYFC VERSION 1.0)

########################
# Global configuration #
########################

# CMake modules
include(CMakeDependentOption)
include(CMakePushCheckState)
include(CheckIncludeFiles)
include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)
include(CheckTypeSize)
include(CheckSymbolExists)

#
# Check if the system is Ubuntu
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    # Check if it's Ubuntu
    if(EXISTS "/etc/lsb-release")
        file(READ "/etc/lsb-release" LSB_RELEASE_CONTENTS)
        string(FIND "${LSB_RELEASE_CONTENTS}" "Ubuntu" IS_UBUNTU)
        if(IS_UBUNTU GREATER -1)
            message("Compiling on Ubuntu")
            # Add Ubuntu-specific configurations or checks here
        endif()
    # Check if it's CentOS
    elseif(EXISTS "/etc/redhat-release")
        file(READ "/etc/redhat-release" REDHAT_RELEASE_CONTENTS)
        string(FIND "${REDHAT_RELEASE_CONTENTS}" "CentOS" IS_CENTOS)
        if(IS_CENTOS GREATER -1)
            message("Compiling on CentOS")
            # Add CentOS-specific configurations or checks here
        endif()
    endif()
endif()


# Extra modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(IYFCMacros)

# Always build position-independent-code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# Make the install target depend on the all target (required by vcpkg)
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY OFF)

# Required files and directories
include(GNUInstallDirs)
# Runtime path setup
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Source Tree
set(IYFC_INCLUDES_DIR ${CMAKE_CURRENT_LIST_DIR})
set(IYFC_CONFIG_IN_FILENAME ${CMAKE_CURRENT_LIST_DIR}/cmake/IYFCConfig.cmake.in)
set(IYFC_CONFIG_H_IN_FILENAME ${IYFC_INCLUDES_DIR}/util/config.h.in)

# Build tree
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${OUTLIB_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${OUTLIB_PATH})
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(IYFC_TARGETS_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/cmake/IYFCTargets.cmake)
set(IYFC_CONFIG_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/cmake/IYFCConfig.cmake)
set(IYFC_CONFIG_VERSION_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/cmake/IYFCConfigVersion.cmake)
set(IYFC_CONFIG_H_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/util/config.h)
set(IYFC_THIRDPARTY_DIR ${CMAKE_CURRENT_BINARY_DIR}/thirdparty)

# Set the installation path
set(IYFC_CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/iyfc-${IYFC_VERSION_MAJOR}.${IYFC_VERSION_MINOR})
set(IYFC_INCLUDES_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/iyfc-${IYFC_VERSION_MAJOR}.${IYFC_VERSION_MINOR})

# pkg-config
set(IYFC_PKGCONFIG_IN_DIR ${CMAKE_CURRENT_LIST_DIR}/pkgconfig)
set(IYFC_PKGCONFIG_IN_FILENAME ${IYFC_PKGCONFIG_IN_DIR}/iyfc.pc.in)
set(IYFC_PKGCONFIG_SHARED_IN_FILENAME ${IYFC_PKGCONFIG_IN_DIR}/iyfc_shared.pc.in)
set(IYFC_PKGCONFIG_DIR ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig)
set(IYFC_PKGCONFIG_FILENAME ${IYFC_PKGCONFIG_DIR}/iyfc.pc)
set(IYFC_PKGCONFIG_SHARED_FILENAME ${IYFC_PKGCONFIG_DIR}/iyfc_shared.pc)
set(IYFC_PKGCONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

include(CleanArtifacts)

#########################
# External dependencies #
#########################

# [option] IYFC_BUILD_DEPS (default: OFF)
# Download and build missing dependencies, throw error if disabled.
set(IYFC_BUILD_DEPS_OPTION_STR "Automatically download and build unmet dependencies")
option(IYFC_BUILD_DEPS ${IYFC_BUILD_DEPS_OPTION_STR} OFF)
message(STATUS "IYFC_BUILD_DEPS: ${IYFC_BUILD_DEPS}")

if(IYFC_BUILD_DEPS)
    include(FetchContent)
    mark_as_advanced(FETCHCONTENT_BASE_DIR)
    mark_as_advanced(FETCHCONTENT_FULLY_DISCONNECTED)
    mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED)
    mark_as_advanced(FETCHCONTENT_QUIET)
endif()

#gpu
option(GPU_SUPPORT "Use GPU to accelerate computation" OFF)
message(STATUS "GPU_SUPPORT: ${GPU_SUPPORT}")
if(GPU_SUPPORT)
    add_definitions(-DGPU_SUPPORT)
    set(CMAKE_CUDA_COMPILER "/usr/local/cuda/bin/nvcc")
    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CUDA_ARCHITECTURES 52)
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --expt-extended-lambda -lcurand ")
    project(iyfc  LANGUAGES CXX C CUDA)
    find_package(CUDA REQUIRED)
    include_directories(${CUDA_INCLUDE_DIRS})
    message("CUDA_INCLUDE_DIRS: ${CUDA_INCLUDE_DIRS}")
endif()

# build seal 
if(IYFC_BUILD_DEPS AND NOT GPU_SUPPORT)
    message(STATUS "Microsoft Seal: download ...")
    iyfc_fetch_thirdparty_content(ExternalSeal)
else()
    find_package(SEAL 4.0 REQUIRED)
if(NOT SEAL_FOUND)
    message(FATAL_ERROR "Microsoft SEAL: not found")
else()
    message(STATUS "Microsoft SEAL: found ${SEAL_INCLUDES_INSTALL_DIR}" )
    include_directories(${SEAL_INCLUDES})
endif()
endif()


# #build oepnfhe
if(IYFC_BUILD_DEPS)
    message(STATUS "openfhe: download ...")
    iyfc_fetch_thirdparty_content(ExternalOpenFHE)
else()
    find_package(OpenFHE)
if(NOT OpenFHE_FOUND)
    message(FATAL_ERROR "openfhe: not found")
else()
    include_directories( ${OpenFHE_INCLUDE} )
    include_directories( ${OpenFHE_INCLUDE}/third-party/include )
    include_directories( ${OpenFHE_INCLUDE}/pke )
    include_directories( ${OpenFHE_INCLUDE}/core )
    include_directories( ${OpenFHE_INCLUDE}/binfhe )
    message("OPENMP_INCLUDES" ${OPENMP_INCLUDES} )
    message("OPENMP_LIBRARIES" ${OPENMP_LIBRARIES} )
    message(STATUS "openfhe: found")
endif()
endif()

# Protobuf
if(IYFC_BUILD_DEPS)
    message(STATUS "Protobuf: download ...")
    iyfc_fetch_thirdparty_content(ExternalProtoBuf)
else()
    find_package(Protobuf)
if(NOT Protobuf_FOUND)
    message(FATAL_ERROR "Protobuf: not found")
else()
    message(STATUS "Protobuf: found")
endif()
endif()


#fftw not support cmake 
# if(IYFC_BUILD_DEPS)
# message(STATUS "fftw3: download ...")
#     iyfc_fetch_thirdparty_content(ExternalFFTW)
# else()
# 
option(AUTO_BUILD_FFT "AUTO_BUILD_FFT" OFF)
message(STATUS "AUTO_BUILD_FFT: ${AUTO_BUILD_FFT}")
if(IYFC_BUILD_DEPS AND AUTO_BUILD_FFT)
    if(IS_UBUNTU)
        execute_process(COMMAND sudo apt-get update)
        execute_process(COMMAND sudo apt-get install -y libfftw3-dev)
    elseif(UNIX AND NOT APPLE)
        execute_process(COMMAND sudo yum install -y fftw-devel)
    else()
        message(FATAL_ERROR "Unsupported operating system.")
    endif()
else()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(FFTW REQUIRED fftw3 IMPORTED_TARGET)
 endif()

#rust
option(AUTO_INSTALL_CARGO "AUTO_INSTALL_CARGO" OFF)
message(STATUS "AUTO_INSTALL_CARGO: ${AUTO_INSTALL_CARGO}")
if(IYFC_BUILD_DEPS AND AUTO_INSTALL_CARGO)
    # execute_process(COMMAND sudo snap install rustup)
    if(IS_UBUNTU)
        execute_process(COMMAND sudo apt  install cargo)
    elseif(UNIX AND NOT APPLE)
        execute_process(COMMAND sudo yum install cargo)
    else()
        message(FATAL_ERROR "Unsupported operating system.")
    endif()
endif()

#openmp
option(AUTO_INSTALL_OPENMP "AUTO_INSTALL_OPENMP" OFF)
message(STATUS "AUTO_INSTALL_OPENMP: ${AUTO_INSTALL_OPENMP}")
if(IYFC_BUILD_DEPS AND AUTO_INSTALL_OPENMP)
    if(IS_UBUNTU)
        execute_process(COMMAND sudo apt-get install libomp-dev)
    elseif(UNIX AND NOT APPLE)
        execute_process(COMMAND sudo yum install libgomp)
    else()
        message(FATAL_ERROR "Unsupported operating system.")
    endif()
endif()

include(ExternalProject)
set_directory_properties(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/concrete/libforc)
ExternalProject_Add(
    concrete_header
    DOWNLOAD_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND cargo build COMMAND cargo build --release
    BINARY_DIR "${CMAKE_SOURCE_DIR}/concrete/libforc"
    INSTALL_COMMAND ""
    LOG_BUILD ON)




# [option] IYFC_BUILD_SHARED (default: OFF)
# Build a shared library if set to ON. Build a static library regardlessly.
set(IYFC_BUILD_SHARED_STR "Build shared library")
option(IYFC_BUILD_SHARED ${IYFC_BUILD_SHARED_STR} OFF)
message(STATUS "IYFC_BUILD_SHARED: ${IYFC_BUILD_SHARED}")

# Add source files to library and header files to install
set(IYFC_SOURCE_FILES ${IYFC_SOURCE_FILES} 
    ${CMAKE_CURRENT_LIST_DIR}/comm_include.cpp
    ${CMAKE_CURRENT_LIST_DIR}/iyfc_include.cpp
)
# Install header files to the local include directory
install(
    FILES iyfc_include.h err_code.h comm_include.h 
    DESTINATION ${IYFC_INCLUDES_INSTALL_DIR}
)


add_subdirectory(util)
add_subdirectory(dag)
add_subdirectory(daghandler)
add_subdirectory(decision)
add_subdirectory(seal/alo)
add_subdirectory(seal/comm)
add_subdirectory(openfhe/comm)
add_subdirectory(openfhe/alo)
add_subdirectory(concrete)
add_subdirectory(concrete/libforc)
add_subdirectory(proto)
add_subdirectory(examples)
# Create the config file
configure_file(${IYFC_CONFIG_H_IN_FILENAME} ${IYFC_CONFIG_H_FILENAME})
install(
    FILES ${IYFC_CONFIG_H_FILENAME}
    DESTINATION ${IYFC_INCLUDES_INSTALL_DIR}/util)

if(NOT IYFC_BUILD_SHARED)
    add_library(iyfc STATIC  ${IYFC_SOURCE_FILES})
    iyfc_set_version_filename(iyfc)
    iyfc_set_include_directories(iyfc)
    iyfc_set_version(iyfc)
    iyfc_link_threads(iyfc)
    iyfc_install_target(iyfc IYFCTargets)
    add_dependencies(iyfc concrete_header)
    find_package(OpenMP REQUIRED)
    target_link_libraries(iyfc PUBLIC  OpenMP::OpenMP_CXX m rt ${CMAKE_DL_LIBS}
    debug "${CMAKE_SOURCE_DIR}/concrete/libforc/target/debug/libconcrete_header.a"
    optimized "${CMAKE_SOURCE_DIR}/concrete/libforc/target/release/libconcrete_header.a"
    "-Wl,--exclude-libs,ALL"
    dl
    )
    if(GPU_SUPPORT)
        target_link_libraries(iyfc PUBLIC  "/usr/local/cuda-11.7/lib64/libcudart_static.a")
        target_link_libraries(iyfc PUBLIC SEAL::seal) #gpu_seal
    endif()
    if(IYFC_BUILD_DEPS)
        target_link_libraries(iyfc PRIVATE  OPENFHEpke_static;OPENFHEcore_static;OPENFHEbinfhe_static )
        target_link_libraries(iyfc PUBLIC  fftw3)
        target_link_libraries(iyfc PRIVATE  ${CMAKE_BINARY_DIR}/libprotobuf.a)
        target_include_directories(iyfc PUBLIC $<BUILD_INTERFACE:${openfhe_SOURCE_DIR}/src/pke/include/>)
        target_include_directories(iyfc PUBLIC $<BUILD_INTERFACE:${openfhe_SOURCE_DIR}/src/core/include/>)
        target_include_directories(iyfc PUBLIC $<BUILD_INTERFACE:${openfhe_SOURCE_DIR}/src/core/include/lattice>)
        target_include_directories(iyfc PUBLIC $<BUILD_INTERFACE:${openfhe_SOURCE_DIR}/src/binfhe/include/>)
        target_include_directories(iyfc PUBLIC $<BUILD_INTERFACE:${openfhe_SOURCE_DIR}/third-party/cereal/include/>)
        target_include_directories(iyfc PUBLIC $<BUILD_INTERFACE:${self_protobuf_SOURCE_DIR}/src>)
        if(NOT GPU_SUPPORT)
            target_link_libraries(iyfc PRIVATE  seal)
            target_include_directories(iyfc PUBLIC $<BUILD_INTERFACE:${seal_SOURCE_DIR}/native/src>)
            target_include_directories(iyfc PUBLIC $<BUILD_INTERFACE:${seal_BINARY_DIR}/native/src>)
            target_include_directories(iyfc PUBLIC $<BUILD_INTERFACE:${seal_BINARY_DIR}/thirdparty/msgsl-src/include>)
        endif()

    else()
        target_link_libraries(iyfc PUBLIC SEAL::seal)
        target_link_libraries(iyfc PUBLIC protobuf::libprotobuf)
        target_link_libraries(iyfc PUBLIC fftw3)
        target_link_libraries(iyfc PUBLIC ${OpenFHE_STATIC_LIBRARIES})
    endif()
#build shared
else()
    # Provide to partners for using dynamic shared object (so)
    add_library(iyfc_shared SHARED ${IYFC_SOURCE_FILES})
    iyfc_set_version(iyfc_shared)
    set_target_properties(iyfc_shared PROPERTIES OUTPUT_NAME iyfc)
    iyfc_set_include_directories(iyfc_shared)
    iyfc_set_version(iyfc_shared)
    iyfc_link_threads(iyfc_shared)
    iyfc_install_target(iyfc_shared IYFCTargets)
    add_dependencies(iyfc_shared concrete_header)
    if(GPU_SUPPORT)
        target_link_libraries(iyfc_shared PUBLIC  "/usr/local/cuda-11.7/lib64/libcudart.so.11.0")
        target_link_libraries(iyfc_shared PUBLIC SEAL::seal)
    endif()
    find_package(OpenMP REQUIRED)
    target_link_libraries(iyfc_shared PUBLIC OpenMP::OpenMP_CXX m rt ${CMAKE_DL_LIBS}
    debug "${CMAKE_SOURCE_DIR}/concrete/libforc/target/debug/libconcrete_header.a"
    optimized "${CMAKE_SOURCE_DIR}/concrete/libforc/target/release/libconcrete_header.a"
    "-Wl,--exclude-libs,ALL"
    dl
    )
    if(IYFC_BUILD_DEPS)
        target_link_libraries(iyfc_shared PRIVATE  OPENFHEpke_static;OPENFHEcore_static;OPENFHEbinfhe_static )
        target_link_libraries(iyfc_shared PUBLIC  fftw3)
        target_link_libraries(iyfc_shared PRIVATE  ${CMAKE_BINARY_DIR}/libprotobuf.a)
        target_include_directories(iyfc_shared PUBLIC $<BUILD_INTERFACE:${openfhe_SOURCE_DIR}/src/pke/include/>)
        target_include_directories(iyfc_shared PUBLIC $<BUILD_INTERFACE:${openfhe_SOURCE_DIR}/src/core/include/>)
        target_include_directories(iyfc_shared PUBLIC $<BUILD_INTERFACE:${openfhe_SOURCE_DIR}/src/core/include/lattice>)
        target_include_directories(iyfc_shared PUBLIC $<BUILD_INTERFACE:${openfhe_SOURCE_DIR}/src/binfhe/include/>)
        target_include_directories(iyfc_shared PUBLIC $<BUILD_INTERFACE:${openfhe_SOURCE_DIR}/third-party/cereal/include/>)
        target_include_directories(iyfc_shared PUBLIC $<BUILD_INTERFACE:${self_protobuf_SOURCE_DIR}/src>)
        if(NOT GPU_SUPPORT)
            target_link_libraries(iyfc_shared PRIVATE  seal)
            target_include_directories(iyfc_shared PUBLIC $<BUILD_INTERFACE:${seal_SOURCE_DIR}/native/src>)
            target_include_directories(iyfc_shared PUBLIC $<BUILD_INTERFACE:${seal_BINARY_DIR}/native/src>)
            target_include_directories(iyfc_shared PUBLIC $<BUILD_INTERFACE:${seal_BINARY_DIR}/thirdparty/msgsl-src/include>)
        endif()
    else()
        target_link_libraries(iyfc_shared PUBLIC SEAL::seal)
        target_link_libraries(iyfc_shared PUBLIC protobuf::libprotobuf)
        target_link_libraries(iyfc_shared PUBLIC fftw3)
        target_link_libraries(iyfc_shared PUBLIC ${OpenFHE_STATIC_LIBRARIES})
    endif()
endif()


# Add standard alias
if(TARGET iyfc)
    add_library(IYFC::iyfc ALIAS iyfc)
endif()
if(TARGET iyfc_shared)
    add_library(IYFC::iyfc_shared ALIAS iyfc_shared)
endif()

# target_compile_options(iyfc PRIVATE -w)

#################################
# Installation and CMake config #
#################################

# Create the CMake config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${IYFC_CONFIG_IN_FILENAME} ${IYFC_CONFIG_FILENAME}
    INSTALL_DESTINATION ${IYFC_CONFIG_INSTALL_DIR})
# Install the export
install(
    EXPORT IYFCTargets
    NAMESPACE IYFC::
    DESTINATION ${IYFC_CONFIG_INSTALL_DIR})

# Version file; we require exact version match for downstream
write_basic_package_version_file(
    ${IYFC_CONFIG_VERSION_FILENAME}
    VERSION ${IYFC_VERSION}
    COMPATIBILITY SameMinorVersion)

# Install config and module files
install(
    FILES
        ${IYFC_CONFIG_FILENAME}
        ${IYFC_CONFIG_VERSION_FILENAME}
    DESTINATION ${IYFC_CONFIG_INSTALL_DIR})

export(
    EXPORT IYFCTargets
    NAMESPACE IYFC::
    FILE ${IYFC_TARGETS_FILENAME})

##############
# pkg-config #
##############
# set(IYFC_PKGCONFIG_SEAL_REQUIRES_STRING "libseal > ${SEAL_VERSION_STRING}")
# # set(IYFC_PKGCONFIG_OPENFHE_REQUIRES_STRING "zlib = ${ZLIB_VERSION_STRING}")
# # set(IYFC_PKGCONFIG_CONCRETE_REQUIRES_STRING "zlib = ${ZLIB_VERSION_STRING}")
# # set(IYFC_PKGCONFIG_FFT_REQUIRES_STRING "zlib = ${ZLIB_VERSION_STRING}")
# # set(IYFC_PKGCONFIG_PROTOBUF_REQUIRES_STRING "zlib = ${ZLIB_VERSION_STRING}")
# Create the pkg-config file
if(TARGET iyfc)
    configure_file(${IYFC_PKGCONFIG_IN_FILENAME} ${IYFC_PKGCONFIG_FILENAME} @ONLY)
endif()
if(TARGET iyfc_shared)
    configure_file(${IYFC_PKGCONFIG_SHARED_IN_FILENAME} ${IYFC_PKGCONFIG_SHARED_FILENAME} @ONLY)
endif()

# Install the pkg-config files
if(EXISTS ${IYFC_PKGCONFIG_FILENAME})
    install(
        FILES ${IYFC_PKGCONFIG_FILENAME}
        DESTINATION ${IYFC_PKGCONFIG_INSTALL_DIR})
endif()
if(EXISTS ${IYFC_PKGCONFIG_SHARED_FILENAME})
    install(
        FILES ${IYFC_PKGCONFIG_SHARED_FILENAME}
        DESTINATION ${IYFC_PKGCONFIG_INSTALL_DIR})
endif()



##################
# iyfc pybind11#
##################

include_directories( include /usr/local/include/pybind11)

##################
# iyfc C++ tests #
##################

set(IYFC_BUILD_TESTS_OPTION_STR "Build C++ tests for  iyfc")
option(IYFC_BUILD_TESTS ${IYFC_BUILD_TESTS_OPTION_STR} OFF)
message(STATUS "IYFC_BUILD_TESTS: ${IYFC_BUILD_TESTS}")

if(IYFC_BUILD_TESTS)
    add_subdirectory(tests)
endif()

#######################
# iyfc C++ benchmarks #
#######################

# [option] IYFC_BUILD_BENCH
set(IYFC_BUILD_BENCH_OPTION_STR "Build C++ benchmarks for Microsoft SEAL")
option(IYFC_BUILD_BENCH ${IYFC_BUILD_BENCH_OPTION_STR} OFF)
message(STATUS "IYFC_BUILD_BENCH: ${IYFC_BUILD_BENCH}")

if(IYFC_BUILD_BENCH)
    add_subdirectory(benchmark)
endif()

#######################
# generate document   #
#######################

# [option] IYFC_BUILD_DOCS (default: OFF)
# Build documentation if set to ON.
set(IYFC_BUILD_DOCS_OPTION_STR "Build documentation")
option(IYFC_BUILD_DOCS ${IYFC_BUILD_DOCS_OPTION_STR} OFF)
message(STATUS "IYFC_BUILD_DOCS: ${IYFC_BUILD_DOCS}")

if(IYFC_BUILD_DOCS)
    add_subdirectory(docs)
endif()


