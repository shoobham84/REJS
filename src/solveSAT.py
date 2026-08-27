import re
from collections import deque

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


def topologicalSort(logic_gates):
    net_driver = {}
    gate_ins = {}
    for i, (inst, cell, ports) in enumerate(logic_gates):
        out = None
        ins = []
        for pin, net in ports.items():
            if pin in ("X", "Y", "HI", "LO"):
                if out is None:
                    out = net
            elif net and net not in ("1'b0", "1'b1"):
                ins.append(net)
        if out:
            net_driver[out] = i
        gate_ins[i] = ins

    n = len(logic_gates)
    in_deg = [0] * n
    deps = [[] for _ in range(n)]
    for i, ins in gate_ins.items():
        for net in ins:
            if net in net_driver:
                j = net_driver[net]
                if j != i:
                    deps[j].append(i)
                    in_deg[i] += 1

    q = deque(i for i in range(n) if in_deg[i] == 0)
    order = []
    while q:
        c = q.popleft()
        order.append(c)
        for d in deps[c]:
            in_deg[d] -= 1
            if in_deg[d] == 0:
                q.append(d)

    visited = set(order)
    for i in range(n):
        if i not in visited:
            order.append(i)

    return [logic_gates[i] for i in order]

