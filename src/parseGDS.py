import pya  #klayout
import os
import math
import json

gdsPath = "puzzle.gds" if os.path.exists("puzzle.gds") else "../puzzle.gds"

layout = pya.Layout() 
layout.read(gdsPath) 

layoutFlat = layout.dup()
topFlat = layoutFlat.top_cell() 
topFlat.flatten(-1, True) #infinite depth (-1), moves every shape into topFlat

dataBaseUnits = layoutFlat.dbu

# li1, met1 - met5 
layers = [(67, 20), (68, 20), (69, 20), (70, 20), (71, 20), (72, 20)]

# mcon, via1 - via4
vias = [(67, 44), (68, 44), (69, 44), (70, 44), (71, 44)]


# merge disjoint polygons
mergedPolygons = {}
spatialGrids = {} 
CELL_SIZE = 2000  # 2.o um gridcell

# index each polygon's bounding box [x1, y1, x2, y2] into spatial buckets
# without spatial indexing there would be 4.2 * 10^8 operations (thats alot)
for layer, data in layers:
    rlayer = pya.Region(topFlat.shapes(layoutFlat.layer(layer, data)))
    rlayer.merge()

    polygonList = []
    grid = {}

    for pidx, plgn in enumerate(rlayer.each()):
        box = plgn.bbox()
        polygonList.append((box, plgn))
        grid_X1 = int(math.floor(box.left / CELL_SIZE))
        grid_Y1 = int(math.floor(box.bottom / CELL_SIZE))
        grid_X2 = int(math.floor(box.right / CELL_SIZE))
        grid_Y2 = int(math.floor(box.top / CELL_SIZE))

        for grid_x in range(grid_X1, grid_X2 + 1):
            for grid_y in range(grid_Y1, grid_Y2 + 1):
                grid.setdefault(( grid_x, grid_y ), []).append(pidx)

    mergedPolygons[( layer, data )] = polygonList
    spatialGrids[( layer, data )] = grid

    print(f"Layer {layer}/{data}: {len(polygonList)} merged continuous polygons")

offsets = {}  # flatten 2d layer index + polygon index into single 1d graph node index
totalNodes = 0

for layer, data in layers:
    offsets[( layer, data )] = totalNodes
    totalNodes += len(mergedPolygons[( layer, data )])

print(f"Total graph nodes across 6 layers: {totalNodes}")


class DSU:
    def __init__(self, n):
        self.p = list(range(n))

    def Find(self, i):
        root = i
        while self.p[root] != root:
            root = self.p[root]

        curr = i
        while curr != root:
            next = self.p[curr]
            self.p[curr] = root
            curr = next
        return root

    def Union(self, i, j):
        ri = self.Find(i)
        rj = self.Find(j)
        if ri != rj:
            self.p[ri] = rj

dsu = DSU(totalNodes)

def queryCandidates(grid, bbox):
    gx1 = int(math.floor(bbox.left / CELL_SIZE))
    gy1 = int(math.floor(bbox.bottom / CELL_SIZE))
    gx2 = int(math.floor(bbox.right / CELL_SIZE))
    gy2 = int(math.floor(bbox.top / CELL_SIZE))

    seen = set()

    for gx in range(gx1, gx2 + 1):
        for gy in range(gy1, gy2 + 1):
            for pidx in grid.get((gx, gy), []):
                seen.add(pidx)

    return seen

# connect adjacent layer's interconnects
# 
for idx, (via_lay, via_dt) in enumerate(vias):
    below_lay = layers[idx]
    above_lay = layers[idx + 1]
    
    plist_below = mergedPolygons[below_lay]
    grid_below = spatialGrids[below_lay]
    
    plist_above = mergedPolygons[above_lay]
    grid_above = spatialGrids[above_lay]
    
    via_shapes = topFlat.shapes(layoutFlat.layer(via_lay, via_dt))
    print(f"Tracing {via_shapes.size()} cuts on Layer {via_lay}/{via_dt} ({below_lay[0]} <-> {above_lay[0]})...")
    
    bridged = 0
    for s in via_shapes.each():
        vb = s.bbox()
        vr = pya.Region(vb)
        
        # find below
        hit_b = None
        for pidx in queryCandidates(grid_below, vb):
            pb, poly = plist_below[pidx]
            if pb.touches(vb) and not (pya.Region(poly) & vr).is_empty():
                hit_b = pidx
                break
        if hit_b is None: continue
        
        # find above
        hit_a = None
        for pidx in queryCandidates(grid_above, vb):
            pb, poly = plist_above[pidx]
            if pb.touches(vb) and not (pya.Region(poly) & vr).is_empty():
                hit_a = pidx
                break
        if hit_a is None: continue
        
        node_b = offsets[below_lay] + hit_b
        node_a = offsets[above_lay] + hit_a
        dsu.Union(node_b, node_a)
        bridged += 1

    print(f"-> Bridged {bridged} / {via_shapes.size()} cuts")

print("Done tracin layers----------------------------")

root_to_net = {}
net_counter = 0
def get_net(node):
    global net_counter
    r = dsu.Find(node)
    if r not in root_to_net:
        root_to_net[r] = net_counter
        net_counter += 1
    return root_to_net[r]

# map cell instance pins
jsonPath = "outputs/parsedCells.json" if os.path.exists("outputs/parsedCells.json") else "../outputs/parsedCells.json"
with open(jsonPath) as f: p1 = json.load(f)

top_orig = layout.top_cell()
inst_pins = {}
grid_li1 = spatialGrids[(67, 20)]
plist_li1 = mergedPolygons[(67, 20)]

for inst in top_orig.each_inst():
    c = inst.cell
    trans = inst.cplx_trans
    ix, iy = trans.disp.x*dataBaseUnits, trans.disp.y*dataBaseUnits
    rot_deg = trans.angle
    is_mir = trans.is_mirror()

    matching = [i for i in p1["cells"] if abs(i["x"] - ix) < 0.001 and abs(i["y"] - iy) < 0.001 and i["cell_type"] == c.name and abs(i["rotation"] - rot_deg) < 0.01 and i["x_reflection"] == is_mir]
    if not matching: continue

    iid = matching[0]["id"]
    
    for s in c.shapes(layout.layer(67, 5)).each():
        if s.is_text():
            t = s.text
            pname = t.string
            tp = t.transformed(trans)
            gx, gy = tp.x, tp.y
            pt_box = pya.Box(gx, gy, gx, gy)
            
            pt = pya.Point(gx, gy)
            for pidx in queryCandidates(grid_li1, pt_box):
                pb, poly = plist_li1[pidx]
                if pb.contains(pt) and poly.inside(pt):
                    nid = get_net(offsets[(67, 20)] + pidx)
                    inst_pins[(iid, pname)] = nid
                    break

print(f"Mapped {len(inst_pins)} instance pins across {len(p1['cells'])} cells")

# map toplevel pads on met3 (70/5)
pad_to_net = {}
grid_m3 = spatialGrids[(70, 20)]
plist_m3 = mergedPolygons[(70, 20)]

for s in top_orig.shapes(layout.layer(70, 5)).each():
    if s.is_text():
        t = s.text
        pname = t.string
        gx, gy = t.x, t.y
        pt = pya.Point(gx, gy)
        pt_box = pya.Box(gx, gy, gx, gy)
        for pidx in queryCandidates(grid_m3, pt_box):
            pb, poly = plist_m3[pidx]
            if pb.contains(pt) and poly.inside(pt):
                pad_to_net[pname] = get_net(offsets[(70, 20)] + pidx)
                break

print(f"Mapped pads: {pad_to_net}")
print(f"Total unique electrical nets: {net_counter}")
