# cmake_minimum_required(VERSION <specify CMake version here>)
project(lab1)

set(CMAKE_CXX_STANDARD 14)

add_executable(lab1 main.cpp Matrix.cpp Matrix.h SlauLDLtCalculator.cpp SlauLDLtCalculator.h)