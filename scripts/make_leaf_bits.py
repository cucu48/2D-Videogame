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
