cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
#Position-Independent Code
add_compile_options(-fPIC)


# add_subdirectory(pybind11)
find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(pyiyfc export_python.cpp)
target_link_libraries(pyiyfc PRIVATE iyfc 
                        $<INSTALL_INTERFACE:include>
                        protobuf::libprotobuf pybind11::embed)