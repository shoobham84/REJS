import pya  #klayout
import os
import math

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

class DSU:
    def __init__(self, n):
        self.p = list(range(n))

    def find(self, i):
        root = i
        while self.p[root] != root:
            root = self.p[root]

        curr = i
        while curr != root:
            next = self.p[curr]
            self.p[curr] = root
            curr = next
        return root


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



