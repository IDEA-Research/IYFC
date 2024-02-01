cmake_minimum_required(VERSION 3.13)

project(IYFCTest VERSION 1.0.0 LANGUAGES CXX C)

    add_executable(iyfctest "")


    if(TARGET IYFC::iyfc)
        target_link_libraries(iyfctest PRIVATE iyfc gtest)
    elseif(TARGET IYFC::iyfc_shared)
        target_link_libraries(iyfctest PRIVATE IYFC::iyfc_shared gtest)
    else()
        message(FATAL_ERROR "Cannot find target iyfc")
    endif()


    # In Debug mode, enable AddressSanitizer (and LeakSanitizer) on Unix-like platforms.
    if(IYFC_DEBUG AND UNIX)
        # On macOS, only AddressSanitizer is enabled.
        # On Linux, LeakSanitizer is enabled by default.
        target_compile_options(iyfctest PUBLIC -fsanitize=address)
        target_link_options(iyfctest PUBLIC -fsanitize=address)
        if(NOT APPLE)
            message(STATUS "Sanitizers enabled: address, leak")
        else()
            message(STATUS "Sanitizers enabled: address")
        endif()
    endif()

target_sources(iyfctest
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/test_runner.cpp
        ${CMAKE_CURRENT_LIST_DIR}/test_comm.cpp
        ${CMAKE_CURRENT_LIST_DIR}/poly_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/decision_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/expr_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/cmp_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/query_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/serialize_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/group_dag_test.cpp
)
