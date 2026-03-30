cmake_minimum_required(VERSION 3.20)
project(Nebula VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3    CONFIG REQUIRED)
find_package(glm      CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)

add_library(glad STATIC vendor/glad/src/glad.c)
target_include_directories(glad PUBLIC vendor/glad/include)

add_subdirectory(engine)
add_subdirectory(game)