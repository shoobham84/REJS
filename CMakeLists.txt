cmake_minimum_required(VERSION 3.23)
project(REJS LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Python3 
    COMPONENTS 
        Interpreter REQUIRED)

add_custom_target(solve
    COMMAND ${CMAKE_COMMAND} -E echo "GDS Layout extraction"
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/parseGDS.py
)
