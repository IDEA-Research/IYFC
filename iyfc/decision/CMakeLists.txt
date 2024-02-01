#cmake_minimum_required(VERSION 3.16)
#set(CMAKE_CXX_STANDARD 17)

#project(decision)

#find_package(SEAL 3.6 REQUIRED)
#find_package(Protobuf)
#include_directories(include /usr/local/include/SEAL-3.6/)

#add_library(decision STATIC



set(IYFC_SOURCE_FILES ${IYFC_SOURCE_FILES} 
    ${CMAKE_CURRENT_LIST_DIR}/alo_decision.cpp
    ${CMAKE_CURRENT_LIST_DIR}/alo_register.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fhe_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/seal_adapter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/concrete_adapter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/openfhe_adapter.cpp
)

set(IYFC_SOURCE_FILES ${IYFC_SOURCE_FILES} PARENT_SCOPE)