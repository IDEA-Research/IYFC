set(IYFC_SOURCE_FILES ${IYFC_SOURCE_FILES} 
    ${CMAKE_CURRENT_LIST_DIR}/levels_checker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mod_switcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encryption_parameter_selector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ckks_config.cpp
    ${CMAKE_CURRENT_LIST_DIR}/seal_ckks_handler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/scales_checker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/eager_waterline_rescaler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/encode_inserter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/lazy_relinearizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/seal_lowering.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/parameter_checker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bfv_handler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bfv_config.cpp
)
set(IYFC_SOURCE_FILES ${IYFC_SOURCE_FILES} PARENT_SCOPE)