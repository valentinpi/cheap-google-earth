cmake_minimum_required(VERSION 3.15)
project(cheap-google-earth)

if(MINGW)
    #set(CMAKE_CXX_FLAGS "-g -O0 -pedantic -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "-O2 -pedantic -Wall -Wextra")
endif(MINGW)

file(GLOB src
    "*.cpp"
    "../dep/lib/*"
)

add_executable(cheap-google-earth ${src})
target_include_directories(cheap-google-earth PRIVATE "../include")
# Ignore warnings from these headers with a SYSTEM header declaration
target_include_directories(cheap-google-earth SYSTEM PRIVATE "../dep/include")
target_link_libraries(cheap-google-earth "-lglfw3 -lopengl32")
