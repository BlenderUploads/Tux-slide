cmake_minimum_required(VERSION 3.10)
project(tux_game CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(tux_game src/main.cpp)
target_link_libraries(tux_game glfw OpenGL::GL GLU)
