# cool findings:

skywater130pdk used

200micrometer - 300micrometer die used

## !All units used will be in micrometer and are implied to be so if no input is given

### register mappings:
y axis coords given at top:
288.32 - 244.80 
success flag logic: inst_450, inst_228, inst_226
4-bit sync cycle states: isnt_234, inst_235, inst_236, inst_237

239.36 - 38.08
> THere are 88 storage registers(11 vertical, 8 FFs)
error latches: inst_4182, inst_4602   @ Y ( 201.28 micrometers)

38.08 - 0.0
input pads: clk, rst_n, enable, I : met3

output pads: success, O\[7:0]   : met3

formatted using AI:

| GDS Layer / Datatype | PDK Layer Name | Physical Function | Connectivity & Rules in Puzzle |
| :--- | :--- | :--- | :--- |
| **67 / 20** | `li1` | Local Interconnect (Titanium Nitride) | Standard cell internal pin contacts (`A, B, X, Y, Q, CLK`) & local routing. |
| **67 / 44** | `mcon` | Metal Contact Via | Connects `li1` up to `met1`. Square vias ($0.17\,\mu\text{m} \times 0.17\,\mu\text{m}$). |
| **68 / 20** | `met1` | Metal 1 (Horizontal tracks) | Intra-row routing and horizontal power rails ($0.48\,\mu\text{m}$ high). |
| **68 / 44** | `via` (`via1`)| Via 1 | Connects `met1` to `met2`. |
| **69 / 20** | `met2` | Metal 2 (Vertical tracks) | Inter-row vertical routing linking flip-flop rows and byte blocks. |
| **69 / 44** | `via2` | Via 2 | Connects `met2` to `met3`. |
| **70 / 20** | `met3` | Metal 3 (Top routing & IO) | Top-level routing and input/output bond pads. |
| **70 / 44** | `via3` | Via 3 | Connects `met3` up to `met4` power trunks. |
| **71 / 20** | `met4` | Metal 4 Power Straps | Vertical VPWR / VGND power distribution trunks ($2.0\,\mu\text{m}$ wide). |
| **72 / 20** | `met5` | Metal 5 Power Grid | Top horizontal VPWR / VGND power distribution bars. |

---

- cell input output (sky130_fd_sc_hd) pins lie on ***layer 67 (li1)***
- standard cell height: 2.72 units, rowpitch: 5.44units
    - odd rows flippped vertically

- `mcon` vias at physical edges of pin polygon
