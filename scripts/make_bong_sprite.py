# Genera images/bong.png: el coleccionable, 2 frames de 16x16 (el segundo con brillo).
# Uso (desde la raiz del repo): python3 scripts/make_bong_sprite.py
from PIL import Image

PAL = {
    '.': (0, 0, 0, 0),
    'O': (16, 48, 24, 255),      # contorno
    'D': (40, 120, 48, 255),     # sombra
    'G': (72, 184, 72, 255),     # verde
    'L': (152, 232, 112, 255),   # luz
    'H': (240, 248, 160, 255),   # reflejo
    'W': (255, 255, 255, 255),   # destello
}

BONG = [
    '.....OOOOO......',
    '.....OHLGO......',
    '.....OLGGO......',
    '....OOOOOOO.....',
    '.....OLGDO......',
    '....OOOOOOO.....',
    '.....OLGDO...OO.',
    '.....OLGDO..OGDO',
    '....OGGGGDOOGDO.',
    '...OGGGGGGGGDO..',
    '..OGHGGGGGGDOO..',
    '..OGHGGGGLGDO...',
    '..OGLGGGGGGDO...',
    '..OGGGGGGGDDO...',
    '...OOGGGDDOO....',
    '..OOOOOOOOOOO...',
]

SPARKLE = {(13, 1): 'W', (12, 1): 'H', (14, 1): 'H', (13, 0): 'H', (13, 2): 'H',
           (3, 6): 'W'}

im = Image.new('RGBA', (32, 16))
for frame in range(2):
    for y, row in enumerate(BONG):
        for x, ch in enumerate(row):
            im.putpixel((frame * 16 + x, y), PAL[ch])
    if frame == 1:
        for (x, y), ch in SPARKLE.items():
            im.putpixel((16 + x, y), PAL[ch])
im.save('images/bong.png')
