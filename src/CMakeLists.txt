cmake_minimum_required(VERSION 3.15)
project(cheap-google-earth)

if(GNU)
    set(CMAKE_CXX_FLAGS "-g -O0 -Wall -Wextra")
endif(GNU)

add_executable(cheap-google-earth "main.cpp" "../lib/glad.c")
target_include_directories(cheap-google-earth PRIVATE "../include")
target_link_libraries(cheap-google-earth "-lglfw3 -lopengl32")
