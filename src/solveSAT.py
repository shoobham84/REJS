import re

def parse_netlist(path="outputs/extracted_netlist.v"):
    with open(path) as f:
        netlistContent = f.read()

    assignments = {}
    flipFlops = []
    logicGates = []

    for line in netlistContent.splitlines():
        line = line.strip()
        if not line or line.startswith("//") or line.startswith("`"):
            continue

        m = re.search(r'(sky130_\w+)\s+(\w+)\s*\((.*?)\);', line)
        if m:
            cell, inst, ports_s = m.group(1), m.group(2), m.group(3)
            ports = {pm.group(1): pm.group(2).strip()
                     for pm in re.finditer(r'\.(\w+)\((.*?)\)', ports_s)}
            bucket = flipFlops if "__df" in cell else logicGates
            bucket.append((inst, cell, ports))
            continue

        m = re.search(r'assign\s+(.*?)\s*=\s*(.*?);', line)
        if m:
            assignments[m.group(1).strip()] = m.group(2).strip()

    pad = {}
    for lhs, rhs in assignments.items():
        if rhs in ("clk", "rst_n", "enable", "I"):
            pad[rhs] = lhs
        elif lhs == 'success' or lhs.startswith("O["):
            pad[lhs] = rhs

    print(f"Parsed {len(logicGates)} combinational gates, {len(flipFlops)} dflipflops, {len(pad)} pad bindings")

    return logicGates, flipFlops, pad
