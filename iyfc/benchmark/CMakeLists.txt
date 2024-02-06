cmake_minimum_required(VERSION 3.13)

project(IYFCBench VERSION 1.0.0 LANGUAGES CXX)

# If not called from root CMakeLists.txt
if(NOT DEFINED IYFC_BUILD_BENCH)
    set(IYFC_BUILD_BENCH ON)

    find_package(IYFC 1.0.0 EXACT REQUIRED)

    # Must define these variables and include macros
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${OUTLIB_PATH})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
    set(IYFC_THIRDPARTY_DIR ${CMAKE_CURRENT_LIST_DIR}/../thirdparty)
    set(THIRDPARTY_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/thirdparty)
    include(FetchContent)
    mark_as_advanced(FETCHCONTENT_BASE_DIR)
    mark_as_advanced(FETCHCONTENT_FULLY_DISCONNECTED)
    mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED)
    mark_as_advanced(FETCHCONTENT_QUIET)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../cmake)
    include(IYFCMacros)
else()
    set(THIRDPARTY_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/../thirdparty)
endif()

if(NOT DEFINED IYFC_BUILD_DEPS)
    # [option] IYFC_BUILD_DEPS (default: ON)
    # Download and build missing dependencies, throw error if disabled.
    set(IYFC_BUILD_DEPS_OPTION_STR "Automatically download and build unmet dependencies")
    option(IYFC_BUILD_DEPS ${IYFC_BUILD_DEPS_OPTION_STR} ON)
endif()

# if IYFC_BUILD_BENCH is ON, use GoogleBenchmark
if(IYFC_BUILD_BENCH)
    if(IYFC_BUILD_DEPS)
        iyfc_fetch_thirdparty_content(ExternalBenchmark)
    else()
        find_package(benchmark REQUIRED)
        if(NOT benchmark_FOUND)
            message(FATAL_ERROR "GoogleBenchmark: not found")
        else()
            message(STATUS "GoogleBenchmark: found")
        endif()
    endif()

    add_executable(iyfcbench)

    target_sources(iyfcbench
        PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/bench.cpp
    )

    if(TARGET iyfc)
        target_link_libraries(iyfcbench PRIVATE iyfc benchmark::benchmark)
    else()
        message(FATAL_ERROR "Cannot find target iyfc ")
    endif()
endif()