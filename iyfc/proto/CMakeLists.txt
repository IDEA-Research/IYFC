cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)

if(IYFC_BUILD_DEPS)
    # protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS known_type.proto iyfc.proto seal.proto concrete.proto openfhe.proto)
    set(GENERATED_CODE_DIR ${CMAKE_CURRENT_BINARY_DIR})
    set(PROTO_SRCS "")
    set(PROTO_HDRS "")
    set(PROTOC ${self_protobuf_BINARY_DIR}/protoc)
    set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")
    
    
    foreach(PROTO_FILE ${PROTO_FILES})
    
        get_filename_component(PROTO_FILE_NAME ${PROTO_FILE} NAME_WE)
        set(PROTO_FILE_SRC ${GENERATED_CODE_DIR}/${PROTO_FILE_NAME}.pb.cc)
        set(PROTO_FILE_HDR ${GENERATED_CODE_DIR}/${PROTO_FILE_NAME}.pb.h)
        add_custom_command(
            OUTPUT ${PROTO_FILE_SRC} ${PROTO_FILE_HDR}
            COMMAND ${PROTOC} --proto_path ${PROTO_DIR} ${PROTO_FILE_NAME}.proto  --cpp_out ${GENERATED_CODE_DIR}  -I ${self_protobuf_SOURCE_DIR}/src
            DEPENDS ${PROTOC} ${PROTO_FILE}
        )
        list(APPEND PROTO_SRCS ${PROTO_FILE_SRC})
        list(APPEND PROTO_HDRS ${PROTO_FILE_HDR})
    endforeach()


    add_library(iyfc_protobuf OBJECT ${PROTO_SRCS} ${PROTO_HDRS})
    target_include_directories(iyfc_protobuf PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${self_protobuf_SOURCE_DIR}/src)
    target_link_libraries(iyfc_protobuf PUBLIC protobuf)
else()
    find_package(Protobuf)
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS known_type.proto iyfc.proto seal.proto concrete.proto openfhe.proto)
    add_library(iyfc_protobuf OBJECT ${PROTO_SRCS} ${PROTO_HDRS})
    target_include_directories(iyfc_protobuf PUBLIC ${CMAKE_CURRENT_BINARY_DIR} )
    target_link_libraries(iyfc_protobuf PUBLIC protobuf::libprotobuf)
endif()




set(IYFC_SOURCE_FILES ${IYFC_SOURCE_FILES} 
    $<TARGET_OBJECTS:iyfc_protobuf>
    ${CMAKE_CURRENT_LIST_DIR}/known_type.cpp
    ${CMAKE_CURRENT_LIST_DIR}/save_load.cpp
    ${CMAKE_CURRENT_LIST_DIR}/iyfc_serialization.cpp
    ${CMAKE_CURRENT_LIST_DIR}/seal_serialization.cpp
    ${CMAKE_CURRENT_LIST_DIR}/concrete_serialization.cpp
    ${CMAKE_CURRENT_LIST_DIR}/openfhe_serialization.cpp
)

set(IYFC_SOURCE_FILES ${IYFC_SOURCE_FILES} PARENT_SCOPE)