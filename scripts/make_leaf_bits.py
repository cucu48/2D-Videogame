# Genera images/leaf_bits.png: 4 trocitos de hoja de 8x8 (pixel art 4x4 a x2)
# que saltan al romper un arbusto. Uso (desde la raiz): python3 scripts/make_leaf_bits.py
from PIL import Image
D = (16, 48, 24, 255); G = (72, 184, 72, 255); L = (152, 232, 112, 255)
frames = [
    ['.GG.', 'GLGD', 'DGG.', '.D..'],
    ['..L.', '.GGD', 'GGD.', 'D...'],
    ['.D..', 'GLG.', '.GGD', '..D.'],
    ['G...', 'LG..', '.GD.', '..D.'],
]
pal = {'.': (0, 0, 0, 0), 'D': D, 'G': G, 'L': L}
im = Image.new('RGBA', (32, 8))
for i, f in enumerate(frames):
    for y, row in enumerate(f):
        for x, c in enumerate(row):
            for dy in range(2):
                for dx in range(2):
                    im.putpixel((i * 8 + x * 2 + dx, y * 2 + dy), pal[c])
im.save('images/leaf_bits.png')
