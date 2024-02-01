
set(IYFC_SOURCE_FILES ${IYFC_SOURCE_FILES} 
    ${CMAKE_CURRENT_LIST_DIR}/type_handler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/reduction_handler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/clean_node_handler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/u32toconst_handler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ckks_rotation_keys_handler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mult_depth_cnt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/node_degree_cnt.cpp
)

set(IYFC_SOURCE_FILES ${IYFC_SOURCE_FILES} PARENT_SCOPE)