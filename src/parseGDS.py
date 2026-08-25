import pya 
import os

gdsPath = "puzzle.gds" if os.path.exists("puzzle.gds") else "../puzzle.gds"

layout = pya.Layout()

layout.read(gdsPath)

layoutFlat = layout.dup()
topFlat = layoutFlat.top_cell()

print("hi")

