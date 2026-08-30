# Reverse Engineering Jane Street's ASIC (2026 Challenge) (REJS)
End-to-end automated reverse-engineering, dynamic waveform simulation, and formal Z3 SAT solver pipeline for the SkyWater 130nm ASIC puzzle.

The write-up for this project/challenge/CTF/thingy can be found below:

https://shoobham84.github.io/REJS/


## Prerequisites & Dependencies

### 1. System Packages
Install `cmake` and `iverilog` (Icarus Verilog) via your system package manager (you can do it!)

### 2. Python Packages
Install the required Python packages (`z3-solver` and `klayout`):
```bash
pip install z3-solver klayout
```


## Setup and run
Clone the repository:
```bash
git clone https://github.com/shoobham84/REJS.git
cd REJS
cmake -B build -S .
cmake --build build --target solve
```


## Running Individual Steps Manually

If you want to run each stage step-by-step:
#### 1. Extract Netlist from GDSII Layout
Extracts physical polygons, bridges 33,323 vias using 3D spatial hashing & DSU, and emits `outputs/extracted_netlist.v`:
```bash
python3 src/parseGDS.py
```

#### 2. Verify Netlist against Golden VCD
Simulates the extracted netlist with Icarus Verilog to verify all 22 golden waveform checkpoints:
```bash
iverilog -g2012 -o outputs/sim_verify src/sky130_cells.v outputs/extracted_netlist.v test/testbench_vcd_verifier.v
vvp outputs/sim_verify
```

#### 3. Run Hardware Easter Egg Testbench
Tests the silicon built-in responses for all-zeros (`"EMPTY SKY"`) and all-ones (`"BIG BANG"`):
```bash
iverilog -g2012 -o outputs/all_zeros_and_ones src/sky130_cells.v outputs/extracted_netlist.v test/all_zeros_and_ones.v
vvp outputs/all_zeros_and_ones
```

#### 4. Solve Hardware Key & Extract Secret Flag
Unrolls the 126-cycle sequential circuit with Microsoft Z3, solves the 121-bit key, proves mathematical uniqueness, and extracts the flag:
```bash
python3 src/solveSAT.py
```

## Output Artifacts
All generated outputs will be written to the `outputs/` directory:

The waveform dumps can be viewed in a software like `gtkwave`

* `outputs/extracted_netlist.v` — Complete gate-level Verilog netlist (728 cells, 741 nets)
* `outputs/sim_output.vcd` — Dynamic simulation waveform dump
* `outputs/all_zeros_and_ones.vcd` — Edge cases (all inputs as zeros and ones) waveform dump
