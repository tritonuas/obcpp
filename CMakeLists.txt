set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_STANDARD_REQUIRED True)

cmake_minimum_required(VERSION 3.10)

project(obcpp VERSION 1.0)

include_directories(obcpp)

add_executable(main main.cpp "bruh (test)/L_+_ratio.cpp")