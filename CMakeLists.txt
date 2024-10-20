cmake_minimum_required(VERSION 3.15)

project("cul")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(${PROJECT_NAME} INTERFACE include/cul/cul.hpp)

target_include_directories(${PROJECT_NAME} PUBLIC include)
