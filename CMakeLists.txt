cmake_minimum_required(VERSION 3.23)
project(REJS NONE)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

find_program(IVERILOG_BIN iverilog)
find_program(VVP_BIN vvp)

if(NOT IVERILOG_BIN OR NOT VVP_BIN)
    message(WARNING "iverilog or vvp not found in PATH.")
endif()

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test")
set(OUTPUTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/outputs")

file(MAKE_DIRECTORY "${OUTPUTS_DIR}")

add_custom_target(solve ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUTS_DIR}"

    COMMAND ${CMAKE_COMMAND} -E echo "-- Extracting netlist from GDS "
    COMMAND ${Python3_EXECUTABLE} "${SRC_DIR}/parseGDS.py"

    COMMAND ${CMAKE_COMMAND} -E echo "-- Verifying netlist against golden VCD "
    COMMAND ${IVERILOG_BIN} -g2012 -o "${OUTPUTS_DIR}/sim_verify"
            "${SRC_DIR}/sky130_cells.v"
            "${OUTPUTS_DIR}/extracted_netlist.v"
            "${TEST_DIR}/testbench_vcd_verifier.v"
    COMMAND ${VVP_BIN} "${OUTPUTS_DIR}/sim_verify"

    COMMAND ${CMAKE_COMMAND} -E echo "-- Running simulation of putting all zeros and ones as inputs "
    COMMAND ${IVERILOG_BIN} -g2012 -o "${OUTPUTS_DIR}/all_zeros_and_ones"
            "${SRC_DIR}/sky130_cells.v"
            "${OUTPUTS_DIR}/extracted_netlist.v"
            "${TEST_DIR}/all_zeros_and_ones.v"
    COMMAND ${VVP_BIN} "${OUTPUTS_DIR}/all_zeros_and_ones"

    COMMAND ${CMAKE_COMMAND} -E echo "-- Solving key and flag via SAT solver "
    COMMAND ${Python3_EXECUTABLE} "${SRC_DIR}/solveSAT.py"

    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Running extraction, simulation, and SAT solving pipeline"
    VERBATIM
)
