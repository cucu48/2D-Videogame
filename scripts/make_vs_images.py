# Genera las imagenes de la pantalla "VS" del nivel del jefe (images/vs_*.png)
# a partir de los sprites del juego. Uso (desde la raiz del repo):
#   python3 scripts/make_vs_images.py
from PIL import Image, ImageDraw, ImageFont

W, H = 320, 304            # pantalla virtual del nivel (mapa 320x288 + HUD 16)
SEAM_TOP, SEAM_BOTTOM = 196, 124   # x de la diagonal arriba y abajo
FONT = 'images/bytebounce.medium.ttf'

BAYER = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]


def seam_x(y, offset=0):
    return SEAM_TOP + (SEAM_BOTTOM - SEAM_TOP) * y / (H - 1) + offset


def dithered(palette, value_fn, block=2):
    """Fondo pixel-art: value_fn(x, y) en [0,1] se cuantiza a la paleta con Bayer."""
    im = Image.new('RGBA', (W, H))
    px = im.load()
    n = len(palette) - 1
    for by in range(0, H, block):
        for bx in range(0, W, block):
            v = max(0.0, min(1.0, value_fn(bx, by))) * n
            i = int(v)
            frac = v - i
            t = (BAYER[(by // block) % 4][(bx // block) % 4] + 0.5) / 16
            c = palette[min(n, i + (1 if frac > t else 0))]
            for y in range(by, min(H, by + block)):
                for x in range(bx, min(W, bx + block)):
                    px[x, y] = c
    return im


def streaks(im, color, seed, slope, count):
    d = ImageDraw.Draw(im)
    import random
    r = random.Random(seed)
    for _ in range(count):
        x = r.randrange(-80, W)
        y = r.randrange(0, H)
        ln = r.randrange(20, 70)
        w = r.choice([2, 2, 4])
        d.line([(x, y), (x + ln, y + ln * slope)], fill=color, width=w)


def text_img(txt, size, fill, outline, outline2=None, scale=2):
    font = ImageFont.truetype(FONT, size)
    l, t, r, b = font.getbbox(txt)
    pad = 3
    im = Image.new('RGBA', (r - l + 2 * pad, b - t + 2 * pad), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    rings = [(outline2, 2)] if outline2 else []
    rings.append((outline, 1))
    for col, rad in rings:
        for dx in range(-rad, rad + 1):
            for dy in range(-rad, rad + 1):
                d.text((pad - l + dx, pad - t + dy), txt, font=font, fill=col)
    d.text((pad - l, pad - t), txt, font=font, fill=fill)
    # sin semitransparencias: el shader descarta alpha < 0.5
    px = im.load()
    for y in range(im.height):
        for x in range(im.width):
            c = px[x, y]
            px[x, y] = (c[0], c[1], c[2], 255) if c[3] >= 128 else (0, 0, 0, 0)
    return im.resize((im.width * scale, im.height * scale), Image.NEAREST)


def sprite(path, box, scale):
    s = Image.open(path).convert('RGBA').crop(box)
    return s.resize((s.width * scale, s.height * scale), Image.NEAREST)


def mask(left):
    m = Image.new('L', (W, H), 0)
    d = ImageDraw.Draw(m)
    if left:
        d.polygon([(0, 0), (seam_x(0, 3), 0), (seam_x(H - 1, 3), H), (0, H)], fill=255)
    else:
        d.polygon([(seam_x(0, -3), 0), (W, 0), (W, H), (seam_x(H - 1, -3), H)], fill=255)
    return m


def border(im, left):
    """Franja negra y otra de color a lo largo de la diagonal."""
    d = ImageDraw.Draw(im)
    for y in range(H):
        x = seam_x(y)
        if left:
            d.line([(x - 4, y), (x + 3, y)], fill=(255, 236, 140, 255))
            d.line([(x - 1, y), (x + 3, y)], fill=(16, 8, 8, 255))
        else:
            d.line([(x - 3, y), (x + 4, y)], fill=(170, 255, 140, 255))
            d.line([(x - 3, y), (x + 1, y)], fill=(16, 8, 8, 255))


def finish(im, left):
    out = Image.new('RGBA', (W, H), (0, 0, 0, 0))
    out.paste(im, (0, 0), mask(left))
    return out


# --- Bomberman (izquierda): fondo de fuego ---
fire = [(40, 0, 8, 255), (112, 8, 8, 255), (200, 32, 16, 255), (240, 104, 24, 255), (255, 200, 64, 255)]
left = dithered(fire, lambda x, y: 1.05 - (((x - 70) / 190.0) ** 2 + ((y - 210) / 230.0) ** 2) ** 0.5)
streaks(left, (255, 220, 120, 255), 1, -0.6, 22)
streaks(left, (60, 0, 8, 255), 2, -0.6, 14)
bomber = sprite('images/player.png', (64, 96, 96, 128), 4)   # de frente
left.alpha_composite(bomber, (12, H - bomber.height - 24))
name = text_img('BOMBERMAN', 16, (255, 255, 255, 255), (16, 8, 8, 255), (230, 40, 24, 255))
left.alpha_composite(name, (6, 6))
border(left, True)
finish(left, True).save('images/vs_left.png')

# --- Trent (derecha): bosque de noche ---
forest = [(8, 4, 20, 255), (24, 16, 56, 255), (24, 64, 48, 255), (48, 136, 56, 255), (152, 224, 96, 255)]
right = dithered(forest, lambda x, y: 1.0 - (((x - 250) / 200.0) ** 2 + ((y - 190) / 230.0) ** 2) ** 0.5)
streaks(right, (180, 255, 140, 255), 3, -0.6, 18)
streaks(right, (8, 4, 20, 255), 4, -0.6, 14)
trent = sprite('images/enemies.png', (0, 352, 64, 416), 3)
right.alpha_composite(trent, (W - trent.width - 4, 20))
name = text_img('TRENT', 16, (255, 255, 255, 255), (16, 8, 8, 255), (40, 150, 56, 255))
right.alpha_composite(name, (W - name.width - 6, H - name.height - 6))
border(right, False)
finish(right, False).save('images/vs_right.png')

# --- "VS" ---
text_img('VS', 32, (255, 214, 48, 255), (200, 16, 16, 255), (16, 8, 8, 255), 3).save('images/vs_text.png')

# fondo negro de 8x8 (se estira)
Image.new('RGBA', (8, 8), (0, 0, 0, 255)).save('images/vs_black.png')
