
#cmake_minimum_required(VERSION 3.16)
#set(CMAKE_CXX_STANDARD 17)

#project(iyfc)

#add_library(util STATIC

set(IYFC_SOURCE_FILES ${IYFC_SOURCE_FILES} 
    ${CMAKE_CURRENT_LIST_DIR}/logging.cpp
    ${CMAKE_CURRENT_LIST_DIR}/timer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/math_util.cpp
    ${CMAKE_CURRENT_LIST_DIR}/clean_util.cpp
)
set(IYFC_SOURCE_FILES ${IYFC_SOURCE_FILES} PARENT_SCOPE)
