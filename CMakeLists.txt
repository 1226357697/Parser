cmake_minimum_required(VERSION 3.10)

project(parser)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(src)
add_subdirectory(third_part)

