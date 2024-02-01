cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)

#project(seal)
#find_package(SEAL 3.6 REQUIRED)
#include_directories(include /usr/local/include/SEAL-3.6/)

add_subdirectory(alo)
add_subdirectory(comm)
