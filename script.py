import pya

layout = pya.Layout()
layout.read("./puzzle.gds")

top = layout.top_cell()

for layer_idx in layout.layer_indices():
    info = layout.get_info(layer_idx)
    shapes = top.shapes(layer_idx)

    texts = [s for s in shapes.each(pya.Shapes.STexts)]
    
    if len(texts) > 0:
        print(f"Layer: {info.layer}/{info.datatype} ")
        for s in texts:
            t = s.text
            x_um = t.x * layout.dbu
            y_um = t.y * layout.dbu
            print(f"Port: `{t.string}` at ({x_um: .3f}, {y_um: .3f})")



