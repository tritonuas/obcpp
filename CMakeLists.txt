set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

cmake_minimum_required(VERSION 3.10)

project(obcpp VERSION 1.0)

add_executable(obcpp src/main.cpp)