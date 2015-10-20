cmake_minimum_required (VERSION 2.6)
project (test)
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 " )

#include_directories("../include")

# add the executable
add_executable(test0 test.cpp)
add_executable(test1 x.cpp y.cpp)
