#cmake_minimum_required(VERSION 3.16)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#project(sealcomm)

#find_package(SEAL 3.6 REQUIRED)
#include_directories(include /usr/local/include/SEAL-3.6/)

#add_library(sealcomm STATIC

set(IYFC_SOURCE_FILES ${IYFC_SOURCE_FILES} 
    ${CMAKE_CURRENT_LIST_DIR}/seal_comm.cpp
    #seal_public.cpp
    ${CMAKE_CURRENT_LIST_DIR}/seal_encoder.cpp
)

set(IYFC_SOURCE_FILES ${IYFC_SOURCE_FILES} PARENT_SCOPE)