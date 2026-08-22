add_executable(parseGDS
    parseGDS.cpp
)

target_include_directories(parseGDS 
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include 
)

target_link_libraries(parseGDS 
    PRIVATE
        gdstk::gdstk
)


