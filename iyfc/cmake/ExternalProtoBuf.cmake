FetchContent_Declare(
    self_protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG        aea4a27 #3.21.11
)
FetchContent_GetProperties(self_protobuf)

if(NOT self_protobuf_POPULATED)
    FetchContent_Populate(self_protobuf)

    set(CMAKE_C_COMPILER ${CMAKE_C_COMPILER} CACHE STRING "" FORCE)
    set(CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER} CACHE STRING "" FORCE)
    set(protobuf_BUILD_SHARED_LIBS  OFF CACHE BOOL "" FORCE)
    set(protobuf_BUILD_TESTS  OFF CACHE BOOL "" FORCE)
    add_subdirectory(
        ${self_protobuf_SOURCE_DIR}
        ${self_protobuf_BINARY_DIR}
        EXCLUDE_FROM_ALL)
endif()



# FetchContent_MakeAvailable(self_protobuf)
