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

find_program(IVERILOG_BIN iverilog)
find_program(VVP_BIN vvp)

add_custom_target(solve
    COMMAND ${CMAKE_COMMAND} -E echo "GDS Layout extraction"
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/parseGDS.py
    COMMENT "Running gds layout extraction"
)

if(IVERILOG_BIN AND VVP_BIN)
    add_custom_command(TARGET solve POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Compiling extracted netlist with iverilog"
        COMMAND ${IVERILOG_BIN} -g2012 -o ${CMAKE_CURRENT_SOURCE_DIR}/outputs/sim_verify
                ${CMAKE_CURRENT_SOURCE_DIR}/src/sky130_cells.v
                ${CMAKE_CURRENT_SOURCE_DIR}/outputs/extracted_netlist.v
                ${CMAKE_CURRENT_SOURCE_DIR}/test/testbench_vcd_verifier.v
        
        COMMAND ${CMAKE_COMMAND} -E echo "Running simulation"
        COMMAND ${VVP_BIN} ${CMAKE_CURRENT_SOURCE_DIR}/outputs/sim_verify
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running dynamic iverilog sim"
    )
else()
    message("iverilog or vvp not found in path. ")
endif()
