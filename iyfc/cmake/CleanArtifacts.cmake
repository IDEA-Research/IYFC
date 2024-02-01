
# Licensed under the MIT license.
# Remove config.h from source tree
if(EXISTS ${IYFC_INCLUDES_DIR}/util/config.h)
    message(STATUS "Removing old ${IYFC_INCLUDES_DIR}/util/config.h")
    file(REMOVE ${IYFC_INCLUDES_DIR}/util/config.h)
endif()