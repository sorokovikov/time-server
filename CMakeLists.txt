cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_COMPILER "clang++")

project(Socket-time-server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(main.out main.cpp)