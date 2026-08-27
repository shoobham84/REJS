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

if(NOT IVERILOG_BIN OR NOT VVP_BIN)
    message(WARNING "iverilog or vvp not found in PATH.")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

add_executable(SAT_solver
    src/netlistParser.cpp
)

add_custom_target(solve
    COMMAND ${CMAKE_COMMAND} -E echo " GDS Layout Extraction "
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/parseGDS.py

    COMMAND ${CMAKE_COMMAND} -E echo " Golden VCD Verification Simulation"
    COMMAND ${IVERILOG_BIN} -g2012 -o ${CMAKE_CURRENT_SOURCE_DIR}/outputs/sim_verify
            ${CMAKE_CURRENT_SOURCE_DIR}/src/sky130_cells.v
            ${CMAKE_CURRENT_SOURCE_DIR}/outputs/extracted_netlist.v
            ${CMAKE_CURRENT_SOURCE_DIR}/test/testbench_vcd_verifier.v
    COMMAND ${VVP_BIN} ${CMAKE_CURRENT_SOURCE_DIR}/outputs/sim_verify

    COMMAND ${CMAKE_COMMAND} -E echo " All Zeros and Ones Simulation "
    COMMAND ${IVERILOG_BIN} -g2012 -o ${CMAKE_CURRENT_SOURCE_DIR}/outputs/all_zeros_and_ones
            ${CMAKE_CURRENT_SOURCE_DIR}/src/sky130_cells.v
            ${CMAKE_CURRENT_SOURCE_DIR}/outputs/extracted_netlist.v
            ${CMAKE_CURRENT_SOURCE_DIR}/test/all_zeros_and_ones.v
    COMMAND ${VVP_BIN} ${CMAKE_CURRENT_SOURCE_DIR}/outputs/all_zeros_and_ones

    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Executing full extraction and dynamic simulation pipeline"
)

add_custom_target(all_zeros_and_ones
    COMMAND ${CMAKE_COMMAND} -E echo "Compiling all_zeros_and_ones test with iverilog"
    COMMAND ${IVERILOG_BIN} -g2012 -o ${CMAKE_CURRENT_SOURCE_DIR}/outputs/all_zeros_and_ones
            ${CMAKE_CURRENT_SOURCE_DIR}/src/sky130_cells.v
            ${CMAKE_CURRENT_SOURCE_DIR}/outputs/extracted_netlist.v
            ${CMAKE_CURRENT_SOURCE_DIR}/test/all_zeros_and_ones.v
    COMMAND ${CMAKE_COMMAND} -E echo "Running all_zeros_and_ones simulation..."
    COMMAND ${VVP_BIN} ${CMAKE_CURRENT_SOURCE_DIR}/outputs/all_zeros_and_ones
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running all zeros and ones dynamic iverilog sim"
)
