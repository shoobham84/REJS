import re
from collections import deque
import z3

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


_P = "sky130_fd_sc_hd__"  # prefix

def eval_gate(cell, gi):
    ct = cell

    # Buf / Inv / Mux
    if ct in (f"{_P}buf_2", f"{_P}clkbuf_4", f"{_P}clkbuf_8", f"{_P}clkbuf_16"):
        return gi("A")
    if ct == f"{_P}inv_2":    return z3.Not(gi("A"))
    if ct == f"{_P}mux2_1":   return z3.If(gi("S"), gi("A1"), gi("A0"))

    # AND
    if ct == f"{_P}and2_2":   return z3.And(gi("A"), gi("B"))
    if ct == f"{_P}and2b_2":  return z3.And(z3.Not(gi("A_N")), gi("B"))
    if ct == f"{_P}and3_2":   return z3.And(gi("A"), gi("B"), gi("C"))
    if ct == f"{_P}and3b_2":  return z3.And(z3.Not(gi("A_N")), gi("B"), gi("C"))
    if ct == f"{_P}and4_2":   return z3.And(gi("A"), gi("B"), gi("C"), gi("D"))
    if ct == f"{_P}and4b_2":  return z3.And(z3.Not(gi("A_N")), gi("B"), gi("C"), gi("D"))
    if ct == f"{_P}and4bb_2": return z3.And(z3.Not(gi("A_N")), z3.Not(gi("B_N")), gi("C"), gi("D"))

    # NAND
    if ct == f"{_P}nand2_2":  return z3.Not(z3.And(gi("A"), gi("B")))
    if ct == f"{_P}nand2b_2": return z3.Not(z3.And(z3.Not(gi("A_N")), gi("B")))
    if ct == f"{_P}nand3_2":  return z3.Not(z3.And(gi("A"), gi("B"), gi("C")))
    if ct == f"{_P}nand3b_2": return z3.Not(z3.And(z3.Not(gi("A_N")), gi("B"), gi("C")))
    if ct == f"{_P}nand4_2":  return z3.Not(z3.And(gi("A"), gi("B"), gi("C"), gi("D")))

    # OR
    if ct == f"{_P}or2_2":    return z3.Or(gi("A"), gi("B"))
    if ct == f"{_P}or3_2":    return z3.Or(gi("A"), gi("B"), gi("C"))
    if ct == f"{_P}or3b_2":   return z3.Or(gi("A"), gi("B"), z3.Not(gi("C_N")))
    if ct == f"{_P}or4_2":    return z3.Or(gi("A"), gi("B"), gi("C"), gi("D"))
    if ct == f"{_P}or4b_2":   return z3.Or(gi("A"), gi("B"), gi("C"), z3.Not(gi("D_N")))
    if ct == f"{_P}or4bb_2":  return z3.Or(gi("A"), gi("B"), z3.Not(gi("C_N")), z3.Not(gi("D_N")))

    # NOR
    if ct == f"{_P}nor2_2":   return z3.Not(z3.Or(gi("A"), gi("B")))
    if ct == f"{_P}nor3_2":   return z3.Not(z3.Or(gi("A"), gi("B"), gi("C")))
    if ct == f"{_P}nor3b_2":  return z3.Not(z3.Or(gi("A"), gi("B"), z3.Not(gi("C_N"))))
    if ct == f"{_P}nor4_2":   return z3.Not(z3.Or(gi("A"), gi("B"), gi("C"), gi("D")))
    if ct == f"{_P}nor4b_2":  return z3.Not(z3.Or(gi("A"), gi("B"), gi("C"), z3.Not(gi("D_N"))))

    # XOR / XNOR
    if ct == f"{_P}xor2_2":   return z3.Xor(gi("A"), gi("B"))
    if ct == f"{_P}xnor2_2":  return z3.Not(z3.Xor(gi("A"), gi("B")))

    # AOI
    if ct == f"{_P}a21o_2":    return z3.Or(z3.And(gi("A1"), gi("A2")), gi("B1"))
    if ct == f"{_P}a21oi_2":   return z3.Not(z3.Or(z3.And(gi("A1"), gi("A2")), gi("B1")))
    if ct == f"{_P}a21bo_2":   return z3.Or(z3.And(gi("A1"), gi("A2")), z3.Not(gi("B1_N")))
    if ct == f"{_P}a21boi_2":  return z3.Not(z3.Or(z3.And(gi("A1"), gi("A2")), z3.Not(gi("B1_N"))))
    if ct == f"{_P}a22o_2":    return z3.Or(z3.And(gi("A1"), gi("A2")), z3.And(gi("B1"), gi("B2")))
    if ct == f"{_P}a22oi_2":   return z3.Not(z3.Or(z3.And(gi("A1"), gi("A2")), z3.And(gi("B1"), gi("B2"))))
    if ct == f"{_P}a211o_2":   return z3.Or(z3.And(gi("A1"), gi("A2")), gi("B1"), gi("C1"))
    if ct == f"{_P}a211oi_2":  return z3.Not(z3.Or(z3.And(gi("A1"), gi("A2")), gi("B1"), gi("C1")))
    if ct == f"{_P}a221o_2":   return z3.Or(z3.And(gi("A1"), gi("A2")), z3.And(gi("B1"), gi("B2")), gi("C1"))
    if ct == f"{_P}a221oi_2":  return z3.Not(z3.Or(z3.And(gi("A1"), gi("A2")), z3.And(gi("B1"), gi("B2")), gi("C1")))
    if ct == f"{_P}a31o_2":    return z3.Or(z3.And(gi("A1"), gi("A2"), gi("A3")), gi("B1"))
    if ct == f"{_P}a31oi_2":   return z3.Not(z3.Or(z3.And(gi("A1"), gi("A2"), gi("A3")), gi("B1")))
    if ct == f"{_P}a32o_2":    return z3.Or(z3.And(gi("A1"), gi("A2"), gi("A3")), z3.And(gi("B1"), gi("B2")))
    if ct == f"{_P}a311o_2":   return z3.Or(z3.And(gi("A1"), gi("A2"), gi("A3")), gi("B1"), gi("C1"))
    if ct == f"{_P}a41oi_2":   return z3.Not(z3.Or(z3.And(gi("A1"), gi("A2"), gi("A3"), gi("A4")), gi("B1")))
    if ct == f"{_P}a2111oi_2": return z3.Not(z3.Or(z3.And(gi("A1"), gi("A2")), gi("B1"), gi("C1"), gi("D1")))

    # OAI
    if ct == f"{_P}o21a_2":    return z3.And(z3.Or(gi("A1"), gi("A2")), gi("B1"))
    if ct == f"{_P}o21ai_2":   return z3.Not(z3.And(z3.Or(gi("A1"), gi("A2")), gi("B1")))
    if ct == f"{_P}o21ba_2":   return z3.And(z3.Or(gi("A1"), gi("A2")), z3.Not(gi("B1_N")))
    if ct == f"{_P}o21bai_2":  return z3.Not(z3.And(z3.Or(gi("A1"), gi("A2")), z3.Not(gi("B1_N"))))
    if ct == f"{_P}o22a_2":    return z3.And(z3.Or(gi("A1"), gi("A2")), z3.Or(gi("B1"), gi("B2")))
    if ct == f"{_P}o22ai_2":   return z3.Not(z3.And(z3.Or(gi("A1"), gi("A2")), z3.Or(gi("B1"), gi("B2"))))
    if ct == f"{_P}o31a_2":    return z3.And(z3.Or(gi("A1"), gi("A2"), gi("A3")), gi("B1"))
    if ct == f"{_P}o31ai_2":   return z3.Not(z3.And(z3.Or(gi("A1"), gi("A2"), gi("A3")), gi("B1")))
    if ct == f"{_P}o32a_2":    return z3.And(z3.Or(gi("A1"), gi("A2"), gi("A3")), z3.Or(gi("B1"), gi("B2")))
    if ct == f"{_P}o32ai_2":   return z3.Not(z3.And(z3.Or(gi("A1"), gi("A2"), gi("A3")), z3.Or(gi("B1"), gi("B2"))))
    if ct == f"{_P}o211a_2":   return z3.And(z3.Or(gi("A1"), gi("A2")), gi("B1"), gi("C1"))
    if ct == f"{_P}o211ai_2":  return z3.Not(z3.And(z3.Or(gi("A1"), gi("A2")), gi("B1"), gi("C1")))
    if ct == f"{_P}o221a_2":   return z3.And(z3.Or(gi("A1"), gi("A2")), z3.Or(gi("B1"), gi("B2")), gi("C1"))
    if ct == f"{_P}o311a_2":   return z3.And(z3.Or(gi("A1"), gi("A2"), gi("A3")), gi("B1"), gi("C1"))
    if ct == f"{_P}o2bb2a_2":  return z3.And(z3.Or(z3.Not(gi("A1_N")), z3.Not(gi("A2_N"))), z3.Or(gi("B1"), gi("B2")))

    raise ValueError(f"Unknown cell: {ct}")


def simulate(sorted_gates, flip_flops, pad, num_cycles, get_input):

    # init dflipflop states at t=0
    dff_q = {}
    for inst, cell, ports in flip_flops:
        dff_q[inst] = z3.BoolVal(True) if "dfstp" in cell else z3.BoolVal(False)

    wire_maps = []

    for t in range(num_cycles):
        rst_n, enable, i_val = get_input(t)
        w = {"1'b0": z3.BoolVal(False), "1'b1": z3.BoolVal(True)}

        if "rst_n" in pad:  w[pad["rst_n"]]  = rst_n
        if "enable" in pad: w[pad["enable"]] = enable
        if "I" in pad:      w[pad["I"]]      = i_val

        for inst, cell, ports in flip_flops:
            qn = ports.get("Q", "")
            if qn:
                w[qn] = dff_q[inst]

        def get_net(net, _w=w, _t=t):
            if not net or net == "1'b0": return z3.BoolVal(False)
            if net == "1'b1":           return z3.BoolVal(True)
            if net not in _w:
                _w[net] = z3.Bool(f"{net}_t{_t}")
            return _w[net]

        for inst, cell, ports in sorted_gates:
            if cell == f"{_P}conb_1":
                if "HI" in ports: w[ports["HI"]] = z3.BoolVal(True)
                if "LO" in ports: w[ports["LO"]] = z3.BoolVal(False)
                continue

            gi = lambda pin, _ports=ports: get_net(_ports.get(pin, ""))
            val = eval_gate(cell, gi)

            for pin in ("X", "Y"):
                if pin in ports:
                    w[ports[pin]] = val
                    break

        wire_maps.append(w)

        if t + 1 < num_cycles:
            next_q = {}
            for inst, cell, ports in flip_flops:
                d_val = get_net(ports.get("D", "1'b0"))
                if "dfrtp" in cell:
                    next_q[inst] = z3.If(z3.Not(rst_n), z3.BoolVal(False), d_val)
                elif "dfstp" in cell:
                    next_q[inst] = z3.If(z3.Not(rst_n), z3.BoolVal(True), d_val)
                else:
                    next_q[inst] = d_val
            dff_q = next_q

    return wire_maps


def solve_key(sorted_gates, flip_flops, pad):
    I_vars = [z3.Bool(f"I_{k}") for k in range(121)]

    def get_input(t):
        if t <= 1:
            return z3.BoolVal(False), z3.BoolVal(False), z3.BoolVal(False)
        rst = z3.BoolVal(True)
        if 3 <= t <= 123:
            return rst, z3.BoolVal(True), I_vars[t - 3]
        return rst, z3.BoolVal(False), z3.BoolVal(False)

    print('unrolling  126 cycles')
    wire_maps = simulate(sorted_gates, flip_flops, pad, 126, get_input)

    success_net = pad.get("success", "net_77")
    success_expr = wire_maps[125][success_net]

    solver = z3.Solver()
    solver.add(success_expr == z3.BoolVal(True))

    print("solving")
    res = solver.check()
    assert res == z3.sat, "UNSAT : no valid key"

    model = solver.model()
    key = [1 if z3.is_true(model.eval(v)) else 0 for v in I_vars]

    print('proving uniqueness')
    block = z3.Or([v != z3.BoolVal(bool(b)) for v, b in zip(I_vars, key)])
    solver.add(block)
    unique = solver.check() == z3.unsat

    return key, unique
