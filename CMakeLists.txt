cmake_minimum_required(VERSION 3.23)
project(REJS LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(GDSTK_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/gdstk/CMakeLists.txt")
    add_subdirectory(gdstk)
endif()

add_executable(parseGDS 
    src/parseGDS.cpp)
target_include_directories(parseGDS    
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(parseGDS
    PRIVATE
        gdstk::gdstk
)


add_executable(extractNets
    src/extractNets.cpp)
target_include_directories(extractNets
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

