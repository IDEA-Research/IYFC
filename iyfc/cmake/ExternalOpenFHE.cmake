FetchContent_Declare(
    openfhe
    GIT_REPOSITORY https://github.com/openfheorg/openfhe-development.git
    GIT_TAG        f6f1b9c#1.1.1
    EXCLUDE_FROM_ALL
)

FetchContent_GetProperties(openfhe)

if(NOT openfhe_POPULATED)
    FetchContent_Populate(openfhe)

    set(BUILD_STATIC ON CACHE BOOL "" FORCE)
    set(BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_UNITTESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(EMSCRIPTEN OFF CACHE BOOL "" FORCE)

    add_subdirectory(
        ${openfhe_SOURCE_DIR}
        ${openfhe_BINARY_DIR}
        EXCLUDE_FROM_ALL)
endif()
include_directories(${openfhe_BINARY_DIR}/src/core)
include_directories(${openfhe_SOURCE_DIR}/src/core/include/lattice)
include_directories(${openfhe_SOURCE_DIR}/src/core/lib)

# set(BUILD_STATIC ON CACHE BOOL "" FORCE)
# set(BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
# set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# set(BUILD_UNITTESTS OFF CACHE BOOL "" FORCE)
# set(BUILD_SHARED OFF CACHE BOOL "" FORCE)
# set(EMSCRIPTEN OFF CACHE BOOL "" FORCE)


# FetchContent_MakeAvailable(openfhe)
# include_directories(${openfhe_BINARY_DIR}/src/core)
# include_directories(${openfhe_SOURCE_DIR}/src/core/include/lattice)
# include_directories(${openfhe_SOURCE_DIR}/src/core/lib)