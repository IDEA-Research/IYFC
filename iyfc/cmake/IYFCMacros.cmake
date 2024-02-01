# Licensed under the MIT license.

# Include a file to fetch thirdparty content
macro(iyfc_fetch_thirdparty_content content_file)
    set(IYFC_FETCHCONTENT_BASE_DIR_OLD ${FETCHCONTENT_BASE_DIR})
    set(FETCHCONTENT_BASE_DIR ${IYFC_THIRDPARTY_DIR} CACHE STRING "" FORCE)
    include(${content_file})
    set(FETCHCONTENT_BASE_DIR ${IYFC_FETCHCONTENT_BASE_DIR_OLD} CACHE STRING "" FORCE)
    unset(IYFC_FETCHCONTENT_BASE_DIR_OLD)
endmacro()

# Include target to given export
macro(iyfc_install_target target export)
    install(TARGETS ${target} EXPORT ${export}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endmacro()


# Set the library filename to reflect version
macro(iyfc_set_version_filename target)
    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME ${target}-${IYFC_VERSION_MAJOR}.${IYFC_VERSION_MINOR})
endmacro()

# Set include directories for build and install interfaces
macro(iyfc_set_include_directories target)
    target_include_directories(${target} PUBLIC
        $<BUILD_INTERFACE:${IYFC_INCLUDES_DIR}>
        $<INSTALL_INTERFACE:${IYFC_INCLUDES_INSTALL_DIR}>)
    target_include_directories(${target} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>
        $<INSTALL_INTERFACE:${IYFC_INCLUDES_INSTALL_DIR}>)
endmacro()


# Set the VERSION property
macro(iyfc_set_version target)
    set_target_properties(${target} PROPERTIES VERSION ${IYFC_VERSION})
endmacro()

# Link a thread library
macro(iyfc_link_threads target)
    # Require thread library
    if(NOT TARGET Threads::Threads)
        set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
        set(THREADS_PREFER_PTHREAD_FLAG TRUE)
        find_package(Threads REQUIRED)
    endif()
    # Link Threads
    target_link_libraries(${target} PUBLIC Threads::Threads)
endmacro()

# Include target to given export
macro(iyfc_install_target target export)
    install(TARGETS ${target} EXPORT ${export}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endmacro()