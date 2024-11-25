set(CMAKE_CXX_STANDARD 23)
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

add_library(memdb INTERFACE memdb.hpp parsing/parser.hpp parsing/conditions.hpp)
target_include_directories(memdb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(memdb PROPERTIES LINKER_LANGUAGE CXX)

add_executable(tests tests.cpp)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/googletest")
target_include_directories(tests PUBLIC "${gtest_SOURCE_DIR}/include")
target_link_libraries(tests gtest gtest_main)
target_link_libraries(tests memdb)