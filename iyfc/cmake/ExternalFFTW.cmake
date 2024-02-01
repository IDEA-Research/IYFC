FetchContent_Declare(
    fftw3
    GIT_REPOSITORY https://github.com/FFTW/fftw3.git
    GIT_TAG        700745c # 3.3.8
)
FetchContent_GetProperties(fftw3)

if(NOT fftw3_POPULATED)
    FetchContent_Populate(fftw3)
    set(DISABLE_FORTRAN ON CACHE BOOL "" FORCE)
    add_subdirectory(
        ${fftw3_SOURCE_DIR}
        ${fftw3_BINARY_DIR}
        EXCLUDE_FROM_ALL)
endif()


# FetchContent_MakeAvailable(fftw3)