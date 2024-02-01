
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)

add_executable(iyfcexamples)

target_sources(iyfcexamples
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/examples.cpp
        ${CMAKE_CURRENT_LIST_DIR}/simple_expr.cpp
        ${CMAKE_CURRENT_LIST_DIR}/avg.cpp
        ${CMAKE_CURRENT_LIST_DIR}/div_v2.cpp
        ${CMAKE_CURRENT_LIST_DIR}/div.cpp
        ${CMAKE_CURRENT_LIST_DIR}/group.cpp
        ${CMAKE_CURRENT_LIST_DIR}/psi_bfv.cpp
        ${CMAKE_CURRENT_LIST_DIR}/query.cpp
        ${CMAKE_CURRENT_LIST_DIR}/serialization.cpp
        ${CMAKE_CURRENT_LIST_DIR}/sort.cpp
        
)
if(NOT IYFC_BUILD_SHARED)
    target_link_libraries(iyfcexamples iyfc)
else()
target_link_libraries(iyfcexamples iyfc_shared)
endif()
target_include_directories(iyfcexamples PUBLIC "../")