from PIL import Image

PAL = {
    '.': (0, 0, 0, 0),
    'O': (16, 48, 24, 255),
    'D': (40, 120, 48, 255),
    'G': (72, 184, 72, 255),
    'L': (152, 232, 112, 255),
    'H': (240, 248, 160, 255),
    'W': (255, 255, 255, 255),
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
