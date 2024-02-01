FetchContent_Declare(
    seal
    GIT_REPOSITORY https://github.com/microsoft/SEAL.git
    GIT_TAG        a0fc0b732f44fa5242593ab488c8b2b3076a5f76 # 4.0
)
FetchContent_GetProperties(seal)

if(NOT seal_POPULATED)
    FetchContent_Populate(seal)
    set(CMAKE_C_COMPILER ${CMAKE_C_COMPILER} CACHE STRING "" FORCE)
    set(CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER} CACHE STRING "" FORCE)
    set(SEAL_THROW_ON_TRANSPARENT_CIPHERTEXT OFF CACHE BOOL "" FORCE)
    set(SEAL_USE_ZLIB OFF CACHE BOOL "" FORCE)
    set(SEAL_USE_ZSTD OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    include_directories(${seal_BINARY_DIR}/native/src/seal)

    add_subdirectory(
        ${seal_SOURCE_DIR}
        ${seal_BINARY_DIR}
        EXCLUDE_FROM_ALL)
endif()


# FetchContent_MakeAvailable(seal)


