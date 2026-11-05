# Genera las imagenes del final secreto (4 bongs + jefe derrotado):
#   images/secret_bg.png   prado al atardecer con Bomberman y Eyti sentados (320x288)
#   images/secret_fg.png   el mismo sin el cielo, para que los creditos salgan de detras de las colinas
#   images/secret_fx.png   bongs animados y bocanadas de humo (celdas de 16x16)
#   images/fade_1..8.png   tramas 8x8 para fundir a negro (fade_8 = negro)
# Uso (desde la raiz del repo): python3 scripts/make_secret_scene.py
import math, random
from PIL import Image, ImageDraw

W, H = 320, 288
HORIZON = 150
BAYER = [[0, 32, 8, 40, 2, 34, 10, 42], [48, 16, 56, 24, 50, 18, 58, 26],
         [12, 44, 4, 36, 14, 46, 6, 38], [60, 28, 52, 20, 62, 30, 54, 22],
         [3, 35, 11, 43, 1, 33, 9, 41], [51, 19, 59, 27, 49, 17, 57, 25],
         [15, 47, 7, 39, 13, 45, 5, 37], [63, 31, 55, 23, 61, 29, 53, 21]]
rnd = random.Random(7)


def thr(x, y):
    return (BAYER[y % 8][x % 8] + 0.5) / 64


def ramp(palette, v, x, y):
    """Color de una rampa de paleta con trama ordenada (v en [0,1])."""
    v = max(0.0, min(1.0, v)) * (len(palette) - 1)
    i = int(v)
    if i >= len(palette) - 1:
        return palette[-1]
    return palette[i + 1] if v - i > thr(x, y) else palette[i]


img = Image.new('RGBA', (W, H))
px = img.load()

# --- cielo: violeta arriba, naranja y amarillo junto al horizonte ---
SKY = [(52, 28, 84), (92, 40, 112), (148, 52, 120), (204, 72, 108),
       (236, 112, 80), (248, 160, 80), (255, 204, 112), (255, 232, 160)]
SUN = (236, 150)
for y in range(HORIZON + 20):
    for x in range(W):
        v = (y / HORIZON) ** 1.15
        d = math.hypot(x - SUN[0], (y - SUN[1]) * 1.4)
        v += max(0.0, 0.35 - d / 420)      # resplandor alrededor del sol
        px[x, y] = ramp(SKY, v, x, y)

# estrellas tenues arriba
for _ in range(26):
    x, y = rnd.randrange(W), rnd.randrange(0, 40)
    px[x, y] = (236, 200, 236, 255)

# sol: disco con bandas, medio escondido tras las colinas
for y in range(SUN[1] - 40, SUN[1] + 1):
    for x in range(SUN[0] - 40, SUN[0] + 41):
        d = math.hypot(x - SUN[0], y - SUN[1])
        if d <= 30:
            band = (SUN[1] - y) % 9
            if y > SUN[1] - 14 and band < 2:   # franjas tipo retro en la parte baja
                continue
            px[x, y] = (255, 246, 200, 255) if d < 26 else (255, 224, 140, 255)
        elif d <= 36 and thr(x, y) < (36 - d) / 6:
            px[x, y] = (255, 214, 130, 255)

# nubes alargadas, iluminadas por debajo
def cloud(cx, cy, w, seed):
    r = random.Random(seed)
    blobs = [(cx + r.randrange(-w // 2, w // 2), cy + r.randrange(-3, 3), r.randrange(5, 10)) for _ in range(9)]
    for y in range(cy - 14, cy + 10):
        for x in range(cx - w // 2 - 12, cx + w // 2 + 12):
            if not (0 <= x < W and 0 <= y < H):
                continue
            inside = any(((x - bx) / (br * 2.2)) ** 2 + ((y - by) / br) ** 2 <= 1 for bx, by, br in blobs)
            if inside and y <= cy + 4:
                lit = (y - (cy - 8)) / 12
                px[x, y] = ramp([(120, 52, 112), (196, 84, 120), (252, 150, 120), (255, 214, 160)], lit, x, y)

cloud(70, 44, 90, 1)
cloud(250, 30, 70, 2)
cloud(150, 92, 120, 3)
cloud(30, 112, 50, 4)

# colinas lejanas (silueta) y cercanas (con borde dorado)
for x in range(W):
    far = HORIZON - 10 + 7 * math.sin(x / 37.0) + 4 * math.sin(x / 13.0 + 1)
    for y in range(int(far), HORIZON + 10):
        px[x, y] = ramp([(84, 44, 96), (104, 56, 100)], (y - far) / 20, x, y)
    near = HORIZON + 2 + 6 * math.sin(x / 51.0 + 2) + 3 * math.sin(x / 17.0)
    for y in range(int(near), HORIZON + 26):
        px[x, y] = (232, 176, 88, 255) if y - near < 1.5 else ramp([(132, 120, 56), (100, 104, 52)], (y - near) / 20, x, y)

# --- prado: verde calido que se oscurece hacia delante ---
GRASS = [(196, 184, 84), (156, 164, 68), (116, 144, 60), (84, 120, 52), (60, 96, 48), (44, 76, 44)]
for y in range(HORIZON + 14, H):
    for x in range(W):
        if px[x, y][3] and y < HORIZON + 26 and px[x, y] != (0, 0, 0, 0) and y < HORIZON + 18:
            continue
        v = (y - HORIZON - 14) / (H - HORIZON - 14)
        v += 0.12 * math.sin(x / 23.0 + y / 9.0)          # ondulacion de la hierba
        v -= max(0.0, 0.25 - math.hypot(x - SUN[0], y - 170) / 500)  # luz del sol rasante
        px[x, y] = ramp(GRASS, v, x, y)

# matas de hierba y flores
for _ in range(260):
    x, y = rnd.randrange(W), rnd.randrange(HORIZON + 22, H)
    h = 2 + (y - HORIZON) // 40
    col = (220, 200, 96, 255) if rnd.random() < 0.5 else (36, 64, 40, 255)
    for k in range(h):
        if 0 <= y - k < H:
            px[x + (k % 2 if x < W - 1 else 0), y - k] = col
for _ in range(70):
    x, y = rnd.randrange(1, W - 1), rnd.randrange(HORIZON + 24, H - 1)
    c = rnd.choice([(255, 248, 220), (255, 200, 72), (248, 132, 168), (200, 160, 255)])
    px[x, y] = c + (255,)
    if y > 210:
        px[x + 1, y] = c + (255,)

d = ImageDraw.Draw(img)


def shadow(cx, cy, w):
    """Sombra alargada hacia la izquierda (el sol esta a la derecha)."""
    for y in range(cy - 3, cy + 4):
        for x in range(cx - w * 2, cx + w // 2):
            if ((x - (cx - w * 0.7)) / (w * 1.25)) ** 2 + ((y - cy) / 3.5) ** 2 <= 1 and thr(x, y) < 0.7:
                if 0 <= x < W and 0 <= y < H:
                    r, g, b, a = px[x, y]
                    px[x, y] = (int(r * 0.6), int(g * 0.6), int(b * 0.75), 255)


def sprite(path, box, scale, flip=False):
    s = Image.open(path).convert('RGBA').crop(box)
    if flip:
        s = s.transpose(Image.FLIP_LEFT_RIGHT)
    s = s.resize((s.width * scale, s.height * scale), Image.NEAREST)
    # luz calida del atardecer: un poco de naranja sobre los colores
    p = s.load()
    for y in range(s.height):
        for x in range(s.width):
            r, g, b, a = p[x, y]
            if a:
                p[x, y] = (min(255, int(r * 0.92 + 26)), min(255, int(g * 0.86 + 12)), int(b * 0.78), 255)
    return s


def put(spr, x, y, sh_w=None):
    if sh_w:
        shadow(x + spr.width // 2, y + spr.height - 2, sh_w)
    img.alpha_composite(spr, (x, y))


EN = 'images/enemies.png'
O, D, G, L, Hh, Wt = (16, 48, 24, 255), (40, 120, 48, 255), (72, 184, 72, 255), (152, 232, 112, 255), (240, 248, 160, 255), (255, 255, 255, 255)
E1, E2 = (255, 120, 32, 255), (255, 220, 96, 255)   # brasa
B1 = (200, 248, 200, 255)                           # burbujas
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
PAL = {'.': (0, 0, 0, 0), 'O': O, 'D': D, 'G': G, 'L': L, 'H': Hh}

# bong pequeno (x2) apoyado en la hierba, para los de atras
def small_bong(x, y, flip=False):
    t = Image.new('RGBA', (16, 16))
    for yy, line in enumerate(BONG):
        for xx, ch in enumerate(line):
            t.putpixel((xx, yy), PAL[ch])
    if flip:
        t = t.transpose(Image.FLIP_LEFT_RIGHT)
    put(sprite_img(t, 2), x, y, 10)


def sprite_img(t, scale):
    t = t.resize((t.width * scale, t.height * scale), Image.NEAREST)
    return t


# --- protagonistas sentados en la hierba, uno frente al otro con un bong
# compartido en medio (pixel art a media resolucion de
# los sprites del juego, x6 en pantalla). Sus bongs se pintan animados desde
# el juego: las posiciones se repiten en SecretScene.cpp ---
# Bomberman sentado de perfil, mirando a Eyti: cabeza del sprite de lado y las
# piernas estiradas hacia delante (la suela roja al final)
BOMBER_SIT = [
    '..AA..AAAAAAA.......',
    'AACCAABBBBBBBA......',
    'AACABBBBBBBBBBA.....',
    '..AABBBBCCCCCCA.....',
    '...ABBBBCBABBBA.....',
    '...ABBBBCBABBBA.....',
    '...ABBBBCCCCCCA.....',
    '....AABABBBBBA......',
    '......ABAAAAA.......',
    '......AACCCAA.......',
    '....AACCABBAA..AAA..',
    '....AACCABBBBBACCCA.',
    '.....ABBBBBBBBACBCA.',
    '.....AAAAAAAAAACCA..',
    '..............AAA...',
]
BOMBER_PAL = {'.': (0, 0, 0, 0), 'A': (0, 40, 24, 255), 'B': (248, 248, 248, 255), 'C': (248, 0, 0, 255)}
# Eyti sentado: la bola apoyada en el suelo (base plana) con los pies hacia delante
EYTI_SIT = [
    '.......AAAAA......',
    '.....AABBBCCAA....',
    '....ABBBBBBBCCA...',
    '...ABBBBBBBBBCCA..',
    '...ABBACBBBBBBCA..',
    '..ABBBAABBBBBBCCA.',
    '..ABBBBBBBBBBACCA.',
    '..AAAAAAABABBBACA.',
    '..ABBBABBBBABBCAA.',
    '..ABBBBBBBBBABCAA.',
    '..ABBBBBBBBBBACCA.',
    '..ABABBBBBBBBCCCA.',
    'AAABBABBBBBBCCCA..',
    'ACCAABBBBBBCCCAA..',
    'ABCCAAAAAAAAAAA...',
    '.AAAA.............',
]
EYTI_PAL = {'.': (0, 0, 0, 0), 'A': (32, 0, 24, 255), 'B': (248, 248, 0, 255), 'C': (232, 104, 40, 255)}


def pixel_sprite(rows, pal, scale):
    s = Image.new('RGBA', (len(rows[0]), len(rows)))
    for y, line in enumerate(rows):
        for x, ch in enumerate(line):
            s.putpixel((x, y), pal[ch])
    s = s.resize((s.width * scale, s.height * scale), Image.NEAREST)
    p = s.load()
    for y in range(s.height):          # luz calida del atardecer
        for x in range(s.width):
            r, g, b, a = p[x, y]
            if a:
                p[x, y] = (min(255, int(r * 0.92 + 26)), min(255, int(g * 0.86 + 12)), int(b * 0.78), 255)
    return s


GROUND = 266   # y del suelo donde se sientan


def sit(spr, x):
    put(spr, x, GROUND - spr.height, 46)


def tufts(x0, x1):
    """Briznas de hierba por delante, tapando un poco la base: sentados EN la hierba."""
    r = random.Random(x0)
    for x in range(x0, x1, 3):
        h = r.randrange(3, 8)
        col = r.choice([(84, 120, 52, 255), (116, 144, 60, 255), (60, 96, 48, 255)])
        tip = (196, 184, 84, 255)
        lean = r.choice([-1, 0, 1])
        for k in range(h):
            xx = x + (lean if k > h // 2 else 0)
            if 0 <= xx < W:
                px[xx, GROUND + 1 - k] = tip if k == h - 1 else col


sit(pixel_sprite(BOMBER_SIT, BOMBER_PAL, 6), 24)    # Bomberman: x 24..144, y 176..266
sit(pixel_sprite(EYTI_SIT, EYTI_PAL, 6), 214)       # Eyti: x 214..322, y 170..266
tufts(28, 150)
tufts(206, 318)

img.save('images/secret_bg.png')

# --- primer plano: lo mismo pero sin el cielo (de las colinas lejanas hacia
# abajo). El juego pinta los creditos entre el fondo y esto, asi que al subir
# salen de detras de las colinas ---
fg = img.copy()
fpx = fg.load()
for x in range(W):
    far = HORIZON - 10 + 7 * math.sin(x / 37.0) + 4 * math.sin(x / 13.0 + 1)
    for y in range(int(far)):
        fpx[x, y] = (0, 0, 0, 0)
fg.save('images/secret_fg.png')

# --- efectos: bongs (normal y espejo) y humo ---
fx = Image.new('RGBA', (128, 32))


def bong_frame(cell, row, bubbles, ember, flip):
    tile = Image.new('RGBA', (16, 16))
    for y, line in enumerate(BONG):
        for x, ch in enumerate(line):
            tile.putpixel((x, y), PAL[ch])
    for (x, y) in bubbles:
        tile.putpixel((x, y), B1)
    if ember:
        tile.putpixel((14, 6), ember)
        tile.putpixel((13, 6), E1)
    if flip:
        tile = tile.transpose(Image.FLIP_LEFT_RIGHT)
    fx.alpha_composite(tile, (cell * 16, row * 16))


for row, flip in ((0, False), (1, True)):
    bong_frame(0, row, [], None, flip)
    bong_frame(1, row, [(5, 12), (8, 10), (6, 7), (7, 4)], E2, flip)
    bong_frame(2, row, [(7, 12), (4, 10), (9, 11), (7, 2)], E1, flip)

# bocanadas de humo: de pequena y densa a grande y deshilachada
SMOKE = [(236, 228, 240, 255), (212, 196, 220, 255), (255, 232, 214, 255)]
for i, (r, keep) in enumerate([(3.0, 1.0), (4.4, 0.95), (6.0, 0.7), (7.4, 0.4)]):
    for y in range(16):
        for x in range(16):
            dd = math.hypot(x - 7.5, y - 7.5)
            if dd <= r and thr(x, y) < keep * (1.2 - dd / (r + 1)):
                c = SMOKE[2] if y < 7.5 - r / 3 else (SMOKE[0] if dd < r * 0.6 else SMOKE[1])
                fx.putpixel((48 + i * 16 + x - 48 + 48, y), c)
fx.save('images/secret_fx.png')

# --- tramas para fundir a negro ---
for k in range(1, 9):
    t = Image.new('RGBA', (8, 8))
    for y in range(8):
        for x in range(8):
            if BAYER[y][x] < k * 8:
                t.putpixel((x, y), (0, 0, 0, 255))
    t.save('images/fade_%d.png' % k)
