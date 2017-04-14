cmake_minimum_required(VERSION 3.7)
project(synacore)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c operations.c operations.h)
add_executable(synacore ${SOURCE_FILES})