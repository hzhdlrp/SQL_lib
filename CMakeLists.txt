cmake_minimum_required(VERSION 3.26)
project(SQL_lib)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(lib)

add_executable(main main.cpp)
target_link_libraries(main INTERFACE memdb)