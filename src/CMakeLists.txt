cmake_minimum_required(VERSION 3.23)

project(rejs LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/gdstk/include
)

link_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/build/gdstk/src
)

