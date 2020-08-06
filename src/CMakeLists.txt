cmake_minimum_required(VERSION 3.15)
project(cheap-google-earth)

if(MINGW)
    set(CMAKE_CXX_FLAGS "-g -O0 -Wall -Wextra")
endif(MINGW)

file(GLOB src
    "*.cpp"
    "../lib/*"
)

add_executable(cheap-google-earth ${src})
target_include_directories(cheap-google-earth PRIVATE "../include")
target_link_libraries(cheap-google-earth "-lglfw3 -lopengl32")
