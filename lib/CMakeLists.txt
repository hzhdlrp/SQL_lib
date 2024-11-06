set(CMAKE_CXX_STANDARD 23)

add_library(memdb STATIC memdb.cpp)
target_include_directories(memdb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
