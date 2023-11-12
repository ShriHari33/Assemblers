cmake_minimum_required(VERSION 3.26)
#project(pass_one_assembler)

set(CMAKE_CXX_STANDARD 17)

add_executable(assembler main.cpp pass_two.h)
add_executable(pass_two pass_two.cpp)
