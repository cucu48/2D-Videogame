# Recorta los 7 enemigos de la hoja "Pocket Bomberman - Characters - Overworld (Color)"
# y genera images/enemies.png (256x256): una fila por enemigo, celdas de 32x32.
#   columnas 0-1: andar mirando a la izquierda
#   columnas 2-3: andar mirando a la derecha
#   columnas 4-5: desaparecer (muerte)
# Uso: python3 scripts/cut_enemy_sprites.py "ruta/a/la/hoja.gif" images
import sys
from PIL import Image

src = Image.open(sys.argv[1]).convert('RGBA')
out_dir = sys.argv[2]

# (y de la fila, segmentos x de cada frame, frames de andar, frames de morir, mira a la derecha)
ENEMIES = [
    (153, [(2,14),(17,29),(32,43),(46,58),(61,72),(75,89),(92,106)], (0,1), (5,6), False),
    (171, [(2,16),(19,33),(36,50),(53,66),(69,83),(86,101),(104,119)], (0,1), (5,6), False),
    (189, [(2,17),(20,35),(38,53),(56,71),(74,89)], (0,1), (3,4), False),
    (207, [(2,17),(20,35),(38,53),(56,71),(74,89),(92,107)], (2,0), (4,5), False),
    (225, [(2,17),(20,35),(38,53),(56,73),(76,91),(94,109),(112,127)], (0,1), (5,6), True),
    (243, [(2,17),(20,35),(38,53),(56,71),(74,89),(92,107),(110,125)], (0,1), (5,6), False),
    (261, [(2,15),(18,30),(33,48),(51,64),(67,82),(85,100),(103,118)], (0,1), (5,6), False),
]

def frame(y, seg):
    a, b = seg
    piece = src.crop((a, y, b + 1, y + 16))
    cell = Image.new('RGBA', (16, 16), (0, 0, 0, 0))
    cell.alpha_composite(piece, ((16 - piece.width) // 2, 0))
    return cell

sheet = Image.new('RGBA', (128, 128), (0, 0, 0, 0))
for row, (y, segs, walk, die, facesRight) in enumerate(ENEMIES):
    w = [frame(y, segs[i]) for i in walk]
    left = w if not facesRight else [f.transpose(Image.FLIP_LEFT_RIGHT) for f in w]
    right = [f.transpose(Image.FLIP_LEFT_RIGHT) for f in left]
    cells = left + right + [frame(y, segs[i]) for i in die]
    for col, c in enumerate(cells):
        sheet.paste(c, (col * 16, row * 16))
sheet.resize((256, 256), Image.NEAREST).save(out_dir + '/enemies.png')
