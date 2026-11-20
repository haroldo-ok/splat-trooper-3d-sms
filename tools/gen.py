#!/usr/bin/env python3
"""Splat Trooper 3D - asset & table generator.

All art is original and drawn procedurally here (no assets from the reference
engine).  Outputs C/asm into ../gen.

Key ideas:
 * Wall textures are 32x32 and vertically symmetric: only the top half
   (32 columns x 16 rows) is stored.  The renderer builds only the top half of
   the view; the bottom half reuses the same VRAM tiles through the name-table
   vertical-flip bit + the sprite palette (which carries floor colours in
   entries 0..2 instead of sky colours).
 * Sprites (enemies, pickups, projectiles) are pre-scaled offline to 8 sizes and
   shown with hardware sprites (8x16 mode).
 * Palette entries 13..15 cycle at run time (barber-pole stripes, portal swirl,
   glowing goo); 0..2 carry the per-level sky/floor gradient.
"""
import math, os, sys
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, '..', 'gen')
os.makedirs(OUT, exist_ok=True)

# ------------------------------------------------------------------ palette
def smsc(r, g, b):
    return r | (g << 2) | (b << 4)

# fixed shared colours (index: rgb 0..3)
BLACK, WHITE, GREY, RED, DKRED, YELLOW, ORANGE, GREEN, DKGREEN, BLUE = 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
CYC_A, CYC_B, CYC_C = 13, 14, 15
FIXED = {
    3: (0, 0, 0), 4: (3, 3, 3), 5: (2, 2, 3), 6: (3, 0, 0), 7: (2, 0, 0),
    8: (3, 3, 0), 9: (3, 2, 0), 10: (1, 3, 0), 11: (0, 2, 0), 12: (0, 1, 3),
}
# 6-step candy rainbow for the cycling entries (13,14,15 use phases 0,2,4)
CYCLE = [(3, 0, 2), (3, 0, 3), (2, 1, 3), (0, 2, 3), (0, 3, 3), (3, 2, 3)]
# per-level sky (pal0 0..2, top->horizon) and floor (pal1 0..2, near->far)
LEVEL_SKYFLOOR = [
    ([(0, 1, 3), (1, 2, 3), (2, 3, 3)], [(1, 3, 1), (0, 2, 0), (0, 1, 0)]),   # sunny day / grass
    ([(2, 0, 2), (3, 1, 2), (3, 2, 1)], [(3, 2, 2), (2, 1, 2), (1, 0, 1)]),   # sunset / candy carpet
    ([(0, 0, 1), (1, 0, 2), (1, 1, 3)], [(2, 2, 2), (1, 1, 2), (0, 0, 1)]),   # night lab / tiles
]
DARK = {3: 3, 4: 5, 5: 12, 6: 7, 7: 3, 8: 9, 9: 7, 10: 11, 11: 3, 12: 3, 13: 13, 14: 14, 15: 15}

def rgb8(c):
    return tuple(v * 85 for v in c)

def full_palette(level=0, which=0):
    sky, floor = LEVEL_SKYFLOOR[level]
    pal = [None] * 16
    for i in range(3):
        pal[i] = (sky if which == 0 else floor)[i]
    for k, v in FIXED.items():
        pal[k] = v
    pal[13], pal[14], pal[15] = CYCLE[0], CYCLE[2], CYCLE[4]
    return pal

# ------------------------------------------------------------------ helpers
def tile_bytes(px):
    """px: 8 rows of 8 colour indices -> 32 bytes SMS planar."""
    out = []
    for row in px:
        for p in range(4):
            b = 0
            for x in range(8):
                if (row[x] >> p) & 1:
                    b |= 0x80 >> x
            out.append(b)
    return out

def c_array(name, data, typ='unsigned char', per=16, extra=''):
    s = '%sconst %s %s[%d] = {\n' % (extra, typ, name, len(data))
    for i in range(0, len(data), per):
        s += '  ' + ','.join(str(v) for v in data[i:i + per]) + ',\n'
    return s + '};\n'

def write(name, text):
    with open(os.path.join(OUT, name), 'w') as f:
        f.write(text)

# ------------------------------------------------------------------ textures
TW, TH = 32, 16   # stored half texture (top half; row 15 touches the horizon)

def tex_brick():
    t = [[RED] * TW for _ in range(TH)]
    for y in range(TH):
        for x in range(TW):
            band = y // 5           # bricks 5 rows high incl. mortar
            yy = y % 5
            off = 8 if band % 2 else 0
            xx = (x + off) % 16
            c = RED
            if yy == 4 or xx == 15:
                c = BLACK
            elif yy == 0 and xx < 14:
                c = ORANGE if xx < 4 else RED     # cartoon highlight
            elif yy == 3 or xx == 14:
                c = DKRED
            t[y][x] = c
    for x in range(TW):
        t[0][x] = BLACK
    return t

def tex_wood():
    t = [[ORANGE] * TW for _ in range(TH)]
    for y in range(TH):
        for x in range(TW):
            px = x % 8
            c = ORANGE
            if px == 0:
                c = BLACK
            elif px == 1:
                c = YELLOW
            elif px == 7:
                c = DKRED
            elif (x // 8 * 5 + y) % 7 == 0 and px in (3, 4):
                c = DKRED          # grain
            t[y][x] = c
        # nails
    for x in range(TW):
        t[0][x] = BLACK
        t[1][x] = DKRED if x % 8 else BLACK
    for bx in range(0, TW, 8):
        t[3][bx + 4] = GREY
        t[3][bx + 3] = BLACK
    return t

def tex_hedge():
    t = [[GREEN] * TW for _ in range(TH)]
    import random
    rnd = random.Random(7)
    for y in range(TH):
        for x in range(TW):
            # scalloped leaf clusters
            cx = (x % 8) - 3.5
            cy = ((y + (4 if (x // 8) % 2 else 0)) % 8) - 3.5
            d = cx * cx + cy * cy
            if d > 13:
                c = DKGREEN
            elif d < 3 and cx < 0 and cy < 0:
                c = YELLOW if d < 1.2 else GREEN
            else:
                c = GREEN
            t[y][x] = c
    for x in range(TW):
        t[0][x] = BLACK
        t[1][x] = DKGREEN
    # glowing goo drips (cycling colour)
    for bx in (5, 18, 27):
        for y in range(2, 2 + 4 + (bx % 5)):
            t[y][bx] = CYC_B
        t[2][bx - 1] = CYC_B
    return t

def tex_stone():
    t = [[GREY] * TW for _ in range(TH)]
    for y in range(TH):
        for x in range(TW):
            xx, yy = x % 16, y % 8
            c = GREY
            if xx == 15 or yy == 7:
                c = BLACK
            elif xx == 0 or yy == 0:
                c = WHITE
            elif xx == 14 or yy == 6:
                c = BLUE
            if 5 <= xx <= 9 and 2 <= yy <= 4 and (xx + yy) % 3 == 0:
                c = BLUE                  # rivet-ish marks
            t[y][x] = c
    for x in range(TW):
        t[0][x] = BLACK
    return t

def tex_candy():
    t = [[WHITE] * TW for _ in range(TH)]
    seq = [CYC_A, CYC_A, CYC_B, CYC_B, CYC_C, CYC_C, WHITE, WHITE]
    for y in range(TH):
        for x in range(TW):
            t[y][x] = seq[(y // 2 + (1 if x % 16 < 8 else 0)) % 8]
    for x in range(TW):
        t[0][x] = BLACK
        t[1][x] = WHITE
    for y in range(TH):
        t[y][0] = BLACK
    return t

def tex_portal():
    t = [[YELLOW] * TW for _ in range(TH)]
    for y in range(TH):
        for x in range(TW):
            # frame + swirl of diamonds centred on (15.5, 15.5 = horizon)
            dx, dy = abs(x - 15.5), abs(y - 15.5)
            if x < 3 or x > 28 or y < 3:
                c = ORANGE if (x in (1, 30) or y == 1) else YELLOW
                if x in (0, 31) or y == 0:
                    c = BLACK
            else:
                ring = int(dx + dy) // 2
                c = [CYC_A, CYC_B, CYC_C][ring % 3]
                if int(dx + dy) in (25, 26):
                    c = BLACK
                if dx + dy < 3:
                    c = WHITE
            t[y][x] = c
    return t

TEXTURES = [tex_brick(), tex_wood(), tex_hedge(), tex_stone(), tex_candy(), tex_portal()]
TEX_NAMES = ['brick', 'wood', 'hedge', 'stone', 'candy', 'portal']

def variants():
    vs = []
    for t in TEXTURES:
        vs.append(t)
        vs.append([[DARK[c] for c in row] for row in t])
    return vs

# ------------------------------------------------------------------ view geometry
NRAYS = 48
FOCAL = 166.0           # px; 192 px wide view -> ~60 deg field of view
HALF = 48               # rows in the top half of the view
NK = 138                # row-table entries per bank
SKYPAGE, SHIFT = 0x90, 0x13

def k_to_hh(k):
    return k if k <= 48 else 48 + (k - 48) * 2

def hh_to_k(hh):
    if hh <= 48:
        return hh
    return min(NK - 1, 48 + (hh - 48 + 1) // 2)

def sky_index(y):
    # three bands with a dithered stripe between them (top -> horizon)
    if y < 13: return 0
    if y < 17: return 1 if (y % 2) else 0
    if y < 29: return 1
    if y < 33: return 2 if (y % 2) else 1
    return 2

def rows_entry(k):
    hh = k_to_hh(k)
    out = []
    top = HALF - hh
    for y in range(HALF):
        if y < top:
            out.append(SKYPAGE + sky_index(y))
        else:
            v = int(((y + 0.5) - top) * 16.0 / hh) if hh else 0
            out.append(0x80 + max(0, min(15, v)))
    return out

def gen_texbanks():
    vs = variants()
    ngroups = (len(vs) + 7) // 8
    banks = []
    for g in range(ngroups):
        b = bytearray(16384)
        for s in range(8):
            v = g * 8 + s
            if v >= len(vs):
                break
            for p in range(16):
                for u in range(32):
                    c = vs[v][p][u]
                    b[p * 256 + s * 32 + u] = c
                    b[(p + SHIFT) * 256 + s * 32 + u] = c << 4
        for c in range(3):
            for i in range(256):
                b[(0x10 + c) * 256 + i] = c
                b[(0x10 + SHIFT + c) * 256 + i] = c << 4
        base = 0x26 * 256
        for k in range(NK):
            e = rows_entry(k)
            b[base + k * 48: base + k * 48 + 48] = bytes(e)
        assert base + NK * 48 <= 16384
        banks.append(b)
    return banks

def gen_tables():
    s = '#include "../src/defs.h"\n'
    # angle units: 1024 per turn
    sint = [int(round(math.sin(a * 2 * math.pi / 1024) * 256)) for a in range(1024)]
    s += c_array('sintab', sint, 'short', 16)
    rayoff, cosoff = [], []
    for i in range(NRAYS):
        ang = math.atan(((i - 23.5) * 4) / FOCAL)
        rayoff.append(int(round(ang * 1024 / (2 * math.pi))))
        cosoff.append(min(255, int(round(math.cos(ang) * 256))))
    s += c_array('rayoff', rayoff, 'signed char', 16)
    s += c_array('cosoff', cosoff, 'unsigned char', 16)
    # hhtab[perp>>3] : half wall height in px (capped 255)
    hh = []
    for i in range(1536):
        perp = i * 8 + 4
        hh.append(min(255, int(round((FOCAL / 2) * 256 / perp))))
    s += c_array('hhtab', hh, 'unsigned char', 32)
    # 256-entry index->4 planes LUT (copied to RAM at boot, page aligned)
    lut = []
    for p in range(4):
        for idx in range(256):
            l, r = idx >> 4, idx & 15
            lut.append((0xF0 if (l >> p) & 1 else 0) | (0x0F if (r >> p) & 1 else 0))
    s += c_array('plane_lut', lut, 'unsigned char', 32)
    # sky tiles for the 6 rows of the top half
    sky = []
    for r in range(6):
        px = [[sky_index(r * 8 + y)] * 8 for y in range(8)]
        sky += tile_bytes(px)
    s += c_array('sky_tiles', sky, 'unsigned char', 32)
    return s

def gen_angbank():
    """per-angle ray record (8 bytes) at 0x8000 + a*8:
       ddx(u16) ddy(u16) |cos|(u8) |sin|(u8) flags(bit0 cos<0, bit1 sin<0) pad
       then at 0xA000: k (row-table index) for perp>>3 (1536 entries)"""
    b = bytearray(16384)
    for a in range(1024):
        ang = a * 2 * math.pi / 1024
        c, sn = math.cos(ang), math.sin(ang)
        def dd(v):
            return 0x7FFF if abs(v) < 1e-6 else min(0x7FFF, int(round(256 / abs(v))))
        ddx, ddy = dd(c), dd(sn)
        ci = int(round(c * 256)); si = int(round(sn * 256))
        fl = (1 if ci < 0 else 0) | (2 if si < 0 else 0)
        rec = [ddx & 255, ddx >> 8, ddy & 255, ddy >> 8, min(255, abs(ci)), min(255, abs(si)), fl, 0]
        b[a * 8:a * 8 + 8] = bytes(rec)
    for i in range(1536):
        perp = i * 8 + 4
        hh = min(255, int(round((FOCAL / 2) * 256 / perp)))
        b[0x2000 + i] = hh_to_k(hh)
    return b

def gen_unrolled_asm():
    a = ['; generated: unrolled column scalers (see tools/gen.py)',
         '\t.module scalers', '\t.area _CODE', '\t.globl dl_body', '\t.globl dr_body', 'dl_body:']
    for _ in range(HALF):
        a += ['\tld a,(bc)', '\tinc bc', '\tadd a,#0x%02X' % SHIFT, '\tld h,a', '\tld a,(hl)', '\tld (de),a', '\tinc e']
    a += ['\tret', 'dr_body:']
    for _ in range(HALF):
        a += ['\tld a,(bc)', '\tinc bc', '\tld h,a', '\tld a,(de)', '\tor (hl)', '\tld (de),a', '\tinc e']
    a += ['\tret', '']
    return '\n'.join(a)

# ------------------------------------------------------------------ cartoon vector art
T = 255   # transparent marker

class Art:
    """Tiny cartoon renderer: shapes in normalised coords, black outlines,
    supersampled then reduced by majority vote (outlines protected)."""
    def __init__(self, w, h, ss=None, ow=1.0):
        self.w, self.h = w, h
        self.ss = ss or (8 if max(w, h) <= 16 else 4)
        self.img = Image.new('L', (w * self.ss, h * self.ss), T)
        self.d = ImageDraw.Draw(self.img)
        self.ow = max(1, int(round(ow * self.ss * (0.8 if max(w, h) < 20 else 1.0))))

    def P(self, x, y):
        return (x * self.w * self.ss, y * self.h * self.ss)

    def ell(self, cx, cy, rx, ry, col, ol=True):
        x0, y0 = self.P(cx - rx, cy - ry)
        x1, y1 = self.P(cx + rx, cy + ry)
        if ol:
            o = self.ow
            self.d.ellipse([x0 - o, y0 - o, x1 + o, y1 + o], fill=BLACK)
        if x1 - x0 >= 1 and y1 - y0 >= 1:
            self.d.ellipse([x0, y0, x1, y1], fill=col)

    def poly(self, pts, col, ol=True):
        pp = [self.P(x, y) for x, y in pts]
        if ol:
            self.d.polygon(pp, fill=BLACK, outline=BLACK, width=2 * self.ow)
        self.d.polygon(pp, fill=col)

    def rect(self, x0, y0, x1, y1, col, ol=True):
        self.poly([(x0, y0), (x1, y0), (x1, y1), (x0, y1)], col, ol)

    def line(self, pts, col, w=1.0):
        pp = [self.P(x, y) for x, y in pts]
        self.d.line(pp, fill=col, width=max(1, int(round(w * self.ss))), joint='curve')

    def star(self, cx, cy, r, col, n=5, inner=0.45, rot=-90, ol=True):
        pts = []
        for i in range(n * 2):
            a = math.radians(rot + i * 180 / n)
            rr = r if i % 2 == 0 else r * inner
            pts.append((cx + rr * math.cos(a) * (self.h / self.w), cy + rr * math.sin(a)))
        self.poly(pts, col, ol)

    def result(self):
        ss = self.ss
        src = self.img.load()
        out = [[T] * self.w for _ in range(self.h)]
        n = ss * ss
        for y in range(self.h):
            for x in range(self.w):
                cnt = {}
                for yy in range(ss):
                    for xx in range(ss):
                        c = src[x * ss + xx, y * ss + yy]
                        cnt[c] = cnt.get(c, 0) + 1
                tcount = cnt.get(T, 0)
                if tcount * 2 > n:
                    continue
                if cnt.get(BLACK, 0) * 100 >= n * 34:
                    out[y][x] = BLACK
                    continue
                best = max((k for k in cnt if k not in (T, BLACK)), key=lambda k: cnt[k], default=BLACK)
                out[y][x] = best
        return out

def splats(a, pts, col=CYC_A):
    for (x, y, r) in pts:
        a.ell(x, y, r, r * 0.8, col, ol=False)
        a.ell(x + r, y - r * 0.6, r * 0.35, r * 0.35, col, ol=False)

# --- Goopy the slime blob
def draw_blob(a, f):
    if f == 4:      # dead: a puddle with dizzy eyes
        a.ell(0.5, 0.86, 0.46, 0.12, GREEN)
        a.ell(0.36, 0.84, 0.07, 0.05, WHITE)
        a.ell(0.62, 0.84, 0.07, 0.05, WHITE)
        a.ell(0.25, 0.9, 0.05, 0.03, CYC_B, ol=False)
        return
    sx, sy = [(1.0, 1.0), (1.12, 0.88), (1.0, 1.08), (1.06, 0.95)][f]
    h = 0.40 * sy
    cy = 0.98 - h
    a.ell(0.5, cy + h * 0.15, 0.44 * sx, h, GREEN)
    a.rect(0.5 - 0.44 * sx, cy + h * 0.15, 0.5 + 0.44 * sx, 0.98, GREEN, ol=False)
    a.line([(0.5 - 0.46 * sx, 0.98), (0.5 + 0.46 * sx, 0.98)], BLACK, 1.0)
    a.ell(0.5, 0.98, 0.44 * sx, 0.03, DKGREEN, ol=False)
    a.ell(0.31, cy - h * 0.25, 0.08, 0.06, WHITE, ol=False)      # shine
    ey = cy + 0.0
    if f == 3:
        for ex in (0.37, 0.63):
            a.line([(ex - 0.07, ey - 0.07), (ex + 0.07, ey + 0.07)], BLACK, 1.0)
            a.line([(ex - 0.07, ey + 0.07), (ex + 0.07, ey - 0.07)], BLACK, 1.0)
        splats(a, [(0.3, 0.75, 0.08), (0.66, 0.68, 0.07)], CYC_A)
    else:
        for ex in (0.37, 0.63):
            a.ell(ex, ey, 0.11, 0.13, WHITE)
            a.ell(ex + 0.02, ey + 0.03, 0.05, 0.06, BLACK, ol=False)
    my = cy + h * 0.55
    if f == 2:
        a.ell(0.5, my + 0.03, 0.2, 0.13, DKRED)
        a.rect(0.36, my - 0.08, 0.44, my - 0.02, WHITE, ol=False)
        a.rect(0.56, my - 0.08, 0.64, my - 0.02, WHITE, ol=False)
        a.ell(0.5, my + 0.1, 0.09, 0.04, RED, ol=False)
    elif f != 3:
        a.line([(0.36, my - 0.02), (0.44, my + 0.04), (0.56, my + 0.04), (0.64, my - 0.02)], BLACK, 1.0)
    a.ell(0.82, cy + h * 0.6, 0.04, 0.06, CYC_B, ol=False)       # glowing goo drop

# --- Fizzle the imp
def draw_imp(a, f):
    if f == 4:
        a.ell(0.5, 0.78, 0.3, 0.18, GREY)
        a.ell(0.35, 0.7, 0.16, 0.12, WHITE, ol=False)
        a.poly([(0.32, 0.92), (0.22, 0.8), (0.38, 0.86)], YELLOW)
        a.poly([(0.68, 0.92), (0.78, 0.8), (0.62, 0.86)], YELLOW)
        return
    leg = [(0.0, 0.0), (0.05, -0.05), (0.0, 0.0), (0.0, 0.0)][f]
    # legs
    a.rect(0.36, 0.78 + leg[0], 0.45, 0.97 + leg[0], DKRED)
    a.rect(0.55, 0.78 + leg[1], 0.64, 0.97 + leg[1], DKRED)
    # tail
    a.line([(0.7, 0.75), (0.86, 0.7), (0.9, 0.55)], DKRED, 1.0)
    a.poly([(0.84, 0.52), (0.96, 0.5), (0.9, 0.42)], DKRED)
    # body + head (one round blob)
    a.ell(0.5, 0.6, 0.27, 0.25, RED)
    a.ell(0.5, 0.38, 0.25, 0.21, RED)
    # horns
    a.poly([(0.3, 0.27), (0.22, 0.05), (0.4, 0.21)], YELLOW)
    a.poly([(0.7, 0.27), (0.78, 0.05), (0.6, 0.21)], YELLOW)
    # eyes
    if f == 3:
        a.line([(0.33, 0.33), (0.45, 0.38)], BLACK, 1.0)
        a.line([(0.67, 0.33), (0.55, 0.38)], BLACK, 1.0)
        splats(a, [(0.42, 0.62, 0.08), (0.62, 0.48, 0.06)], CYC_A)
    else:
        a.ell(0.4, 0.36, 0.08, 0.07, YELLOW)
        a.ell(0.6, 0.36, 0.08, 0.07, YELLOW)
        a.ell(0.41, 0.37, 0.03, 0.04, BLACK, ol=False)
        a.ell(0.59, 0.37, 0.03, 0.04, BLACK, ol=False)
        a.line([(0.3, 0.27), (0.46, 0.31)], BLACK, 1.0)
        a.line([(0.7, 0.27), (0.54, 0.31)], BLACK, 1.0)
    # grin
    a.poly([(0.36, 0.46), (0.64, 0.46), (0.5, 0.56)], WHITE)
    # arms
    if f == 2:
        a.ell(0.2, 0.48, 0.07, 0.07, RED)
        a.line([(0.24, 0.55), (0.2, 0.42)], RED, 1.6)
        a.ell(0.16, 0.24, 0.12, 0.12, ORANGE)
        a.ell(0.16, 0.26, 0.07, 0.07, YELLOW, ol=False)
        a.ell(0.14, 0.18, 0.04, 0.05, CYC_C, ol=False)
    else:
        a.ell(0.22, 0.62 + leg[1], 0.07, 0.08, RED)
    a.ell(0.78, 0.62 + leg[0], 0.07, 0.08, RED)

# --- Klunk the robot
def draw_robot(a, f):
    if f == 4:
        a.rect(0.22, 0.72, 0.78, 0.96, GREY)
        a.rect(0.35, 0.58, 0.62, 0.74, WHITE)
        a.ell(0.48, 0.66, 0.05, 0.04, BLACK, ol=False)
        a.line([(0.7, 0.6), (0.82, 0.48)], BLACK, 1.0)
        a.ell(0.84, 0.46, 0.05, 0.05, CYC_A)
        return
    tilt = 0.03 if f == 3 else 0.0
    # treads
    a.rect(0.18, 0.78, 0.82, 0.97, BLUE)
    for i, wx in enumerate((0.28, 0.5, 0.72)):
        a.ell(wx, 0.875, 0.07, 0.06, GREY)
        if (i + f) % 2:
            a.ell(wx, 0.875, 0.02, 0.02, BLACK, ol=False)
    # body
    a.rect(0.24, 0.42, 0.76, 0.8, GREY)
    a.rect(0.34, 0.52, 0.66, 0.68, BLUE)
    a.ell(0.42, 0.6, 0.03, 0.03, CYC_B, ol=False)
    a.ell(0.5, 0.6, 0.03, 0.03, YELLOW, ol=False)
    a.ell(0.58, 0.6, 0.03, 0.03, RED, ol=False)
    # head
    a.rect(0.3 + tilt, 0.14, 0.7 + tilt, 0.42, WHITE)
    a.line([(0.5, 0.14), (0.5, 0.04)], BLACK, 1.0)
    a.ell(0.5, 0.04, 0.05, 0.05, CYC_A)
    if f == 3:
        a.line([(0.38, 0.24), (0.62, 0.32)], BLACK, 1.2)
        splats(a, [(0.36, 0.6, 0.09), (0.64, 0.3, 0.06)], CYC_A)
    else:
        a.ell(0.5 + tilt, 0.27, 0.13, 0.08, RED if f != 2 else CYC_C)
        a.ell(0.46 + tilt, 0.25, 0.03, 0.025, WHITE, ol=False)
    # arms
    if f == 2:
        a.rect(0.76, 0.48, 0.96, 0.56, GREY)
        a.ell(0.97, 0.52, 0.05, 0.07, CYC_B)
        a.rect(0.04, 0.48, 0.24, 0.56, GREY)
    else:
        sw = 0.04 if f == 1 else -0.04
        a.rect(0.12, 0.45 + sw, 0.24, 0.7 + sw, GREY)
        a.rect(0.76, 0.45 - sw, 0.88, 0.7 - sw, GREY)

def draw_heart(a, f):
    a.ell(0.36, 0.5, 0.2, 0.19, RED)
    a.ell(0.64, 0.5, 0.2, 0.19, RED)
    a.poly([(0.17, 0.56), (0.83, 0.56), (0.5, 0.94)], RED)
    a.ell(0.36, 0.5, 0.18, 0.17, RED, ol=False)
    a.ell(0.64, 0.5, 0.18, 0.17, RED, ol=False)
    a.ell(0.32, 0.45, 0.06, 0.05, WHITE, ol=False)

def draw_ammo(a, f):
    a.rect(0.26, 0.42, 0.74, 0.95, BLUE)
    a.rect(0.26, 0.56, 0.74, 0.8, WHITE, ol=False)
    a.ell(0.5, 0.68, 0.11, 0.08, CYC_A, ol=False)
    a.ell(0.5, 0.42, 0.24, 0.07, GREY)
    a.ell(0.5, 0.41, 0.16, 0.04, CYC_B, ol=False)
    a.ell(0.32, 0.36, 0.06, 0.06, CYC_B)       # splash blob on the lid

def draw_star(a, f):
    a.star(0.5, 0.6, 0.36, YELLOW)
    a.ell(0.43, 0.56, 0.03, 0.05, BLACK, ol=False)
    a.ell(0.57, 0.56, 0.03, 0.05, BLACK, ol=False)
    a.line([(0.42, 0.68), (0.5, 0.72), (0.58, 0.68)], BLACK, 0.8)
    a.ell(0.2, 0.3, 0.05, 0.05, CYC_A, ol=False)
    a.ell(0.82, 0.34, 0.04, 0.04, CYC_C, ol=False)

def draw_fireball(a, f):
    a.ell(0.5, 0.5, 0.36, 0.36, ORANGE)
    a.poly([(0.2, 0.4), (0.06, 0.2), (0.35, 0.25)], ORANGE)
    a.poly([(0.8, 0.4), (0.94, 0.2), (0.65, 0.25)], ORANGE)
    a.ell(0.5, 0.52, 0.22, 0.22, YELLOW, ol=False)
    a.ell(0.5, 0.56, 0.1, 0.1, CYC_C, ol=False)

def draw_plasma(a, f):
    a.ell(0.5, 0.5, 0.36, 0.36, CYC_A)
    a.ell(0.5, 0.5, 0.24, 0.24, CYC_B, ol=False)
    a.ell(0.5, 0.5, 0.12, 0.12, WHITE, ol=False)

def draw_splat(a, f):
    a.ell(0.5, 0.5, 0.3, 0.26, CYC_A)
    for ang in range(0, 360, 60):
        r = math.radians(ang + 15)
        a.ell(0.5 + 0.36 * math.cos(r), 0.5 + 0.34 * math.sin(r), 0.08, 0.08, CYC_A)
    a.ell(0.5, 0.5, 0.28, 0.24, CYC_A, ol=False)
    a.ell(0.42, 0.42, 0.08, 0.06, WHITE, ol=False)

GRAPHICS = []          # (name, fn, frame)
for nm, fn in (('blob', draw_blob), ('imp', draw_imp), ('robot', draw_robot)):
    for fr, fnm in enumerate(('walk0', 'walk1', 'attack', 'hurt', 'dead')):
        GRAPHICS.append(('%s_%s' % (nm, fnm), fn, fr))
for nm, fn in (('heart', draw_heart), ('ammo', draw_ammo), ('star', draw_star),
               ('fireball', draw_fireball), ('plasma', draw_plasma), ('splat', draw_splat)):
    GRAPHICS.append((nm, fn, 0))
SIZES = [8, 12, 16, 20, 24, 32, 40, 48]

def render_graphic(fn, frame, S):
    a = Art(S, S)
    fn(a, frame)
    return a.result()

def sprite_pieces(img, S):
    cols = (S + 7) // 8
    rows = (S + 15) // 16
    gw, gh = cols * 8, rows * 16
    ox, oy = (gw - S) // 2, gh - S
    grid = [[0] * gw for _ in range(gh)]
    for y in range(S):
        for x in range(S):
            c = img[y][x]
            if c != T:
                grid[oy + y][ox + x] = c
    pieces, data = [], []
    for r in range(rows):
        for c in range(cols):
            blk = [grid[r * 16 + yy][c * 8:(c + 1) * 8] for yy in range(16)]
            if any(any(v for v in row) for row in blk):
                pieces.append(c | (r << 4))
                data += tile_bytes(blk[:8]) + tile_bytes(blk[8:])
    return cols, rows, pieces, data

def gen_sprites(first_bank):
    """returns (fixed_c, list_of_bank_c_sources, preview image)"""
    banks = [[]]
    used = [0]
    meta = []
    piece_lists = []
    preview = Image.new('RGB', (sum(SIZES) + 8 * len(SIZES), 52 * len(GRAPHICS)), (60, 60, 60))
    pal = full_palette(0, 1)
    for gi, (nm, fn, fr) in enumerate(GRAPHICS):
        px = 0
        for si, S in enumerate(SIZES):
            img = render_graphic(fn, fr, S)
            for y in range(S):
                for x in range(S):
                    if img[y][x] != T:
                        preview.putpixel((px + x, gi * 52 + 48 - S + y), rgb8(pal[img[y][x]]))
            px += S + 8
            cols, rows, pieces, data = sprite_pieces(img, S)
            if used[-1] + len(data) > 16000:
                banks.append([])
                used.append(0)
            bi = len(banks) - 1
            name = 'spr_%d_%d' % (gi, si)
            banks[bi].append(c_array(name, data))
            meta.append((first_bank + bi, name, len(pieces), cols, rows, 'pcs_%d_%d' % (gi, si)))
            piece_lists.append(c_array('pcs_%d_%d' % (gi, si), pieces or [0], 'unsigned char', 32, 'static '))
            used[-1] += len(data)
    fixed = '#include "../src/defs.h"\n'
    for bi in range(len(banks)):
        fixed += '#include "spr_bank%d.h"\n' % bi
    fixed += ''.join(piece_lists)
    fixed += 'const SprImg spr_imgs[%d] = {\n' % len(meta)
    for (b, name, n, cols, rows, pl) in meta:
        fixed += '  {%d, %s, %d, %d, %d, %s},\n' % (b, name, n, cols, rows, pl)
    fixed += '};\n'
    srcs = []
    for bi, arrs in enumerate(banks):
        srcs.append('\n'.join(arrs))
        hdr = ''.join('extern const unsigned char %s[];\n' % m[1] for m in meta if m[0] == first_bank + bi)
        write('spr_bank%d.h' % bi, hdr)
    return fixed, srcs, preview

# ------------------------------------------------------------------ font (5x7, hand drawn)
GLYPHS = {
 '0': ["01110","10011","10101","10101","11001","10001","01110"],
 '1': ["00100","01100","00100","00100","00100","00100","01110"],
 '2': ["01110","10001","00001","00110","01000","10000","11111"],
 '3': ["11110","00001","00001","01110","00001","00001","11110"],
 '4': ["00010","00110","01010","10010","11111","00010","00010"],
 '5': ["11111","10000","11110","00001","00001","10001","01110"],
 '6': ["00110","01000","10000","11110","10001","10001","01110"],
 '7': ["11111","00001","00010","00100","01000","01000","01000"],
 '8': ["01110","10001","10001","01110","10001","10001","01110"],
 '9': ["01110","10001","10001","01111","00001","00010","01100"],
 'A': ["01110","10001","10001","11111","10001","10001","10001"],
 'B': ["11110","10001","10001","11110","10001","10001","11110"],
 'C': ["01110","10001","10000","10000","10000","10001","01110"],
 'D': ["11100","10010","10001","10001","10001","10010","11100"],
 'E': ["11111","10000","10000","11110","10000","10000","11111"],
 'F': ["11111","10000","10000","11110","10000","10000","10000"],
 'G': ["01110","10001","10000","10111","10001","10001","01111"],
 'H': ["10001","10001","10001","11111","10001","10001","10001"],
 'I': ["01110","00100","00100","00100","00100","00100","01110"],
 'J': ["00111","00010","00010","00010","00010","10010","01100"],
 'K': ["10001","10010","10100","11000","10100","10010","10001"],
 'L': ["10000","10000","10000","10000","10000","10000","11111"],
 'M': ["10001","11011","10101","10101","10001","10001","10001"],
 'N': ["10001","10001","11001","10101","10011","10001","10001"],
 'O': ["01110","10001","10001","10001","10001","10001","01110"],
 'P': ["11110","10001","10001","11110","10000","10000","10000"],
 'Q': ["01110","10001","10001","10001","10101","10010","01101"],
 'R': ["11110","10001","10001","11110","10100","10010","10001"],
 'S': ["01111","10000","10000","01110","00001","00001","11110"],
 'T': ["11111","00100","00100","00100","00100","00100","00100"],
 'U': ["10001","10001","10001","10001","10001","10001","01110"],
 'V': ["10001","10001","10001","10001","10001","01010","00100"],
 'W': ["10001","10001","10001","10101","10101","10101","01010"],
 'X': ["10001","10001","01010","00100","01010","10001","10001"],
 'Y': ["10001","10001","01010","00100","00100","00100","00100"],
 'Z': ["11111","00001","00010","00100","01000","10000","11111"],
 '!': ["00100","00100","00100","00100","00100","00000","00100"],
 '?': ["01110","10001","00001","00110","00100","00000","00100"],
 '.': ["00000","00000","00000","00000","00000","00000","00100"],
 ',': ["00000","00000","00000","00000","00000","00100","01000"],
 ':': ["00000","00100","00000","00000","00000","00100","00000"],
 '-': ["00000","00000","00000","01110","00000","00000","00000"],
 "'": ["00100","00100","01000","00000","00000","00000","00000"],
 '/': ["00001","00010","00010","00100","01000","01000","10000"],
 '%': ["11001","11010","00010","00100","01000","01011","10011"],
 '(': ["00010","00100","01000","01000","01000","00100","00010"],
 ')': ["01000","00100","00010","00010","00010","00100","01000"],
 '=': ["00000","00000","11111","00000","11111","00000","00000"],
 '+': ["00000","00100","00100","11111","00100","00100","00000"],
}

def glyph_pixels(ch, fg, bg, ol=BLACK):
    g = GLYPHS.get(ch)
    px = [[bg] * 8 for _ in range(8)]
    if not g:
        return px
    on = set()
    for y in range(7):
        for x in range(5):
            if g[y][x] == '1':
                on.add((x + 1, y))
    for (x, y) in on:
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1):
                xx, yy = x + dx, y + dy
                if 0 <= xx < 8 and 0 <= yy < 8 and (xx, yy) not in on:
                    px[yy][xx] = ol
    for (x, y) in on:
        px[y][x] = fg
    return px

def draw_text_img(img, x, y, text, fg, ol=BLACK, bg=None):
    """stamp outlined glyphs into an index image (transparent bg unless bg)."""
    for i, ch in enumerate(text):
        g = GLYPHS.get(ch)
        if not g:
            continue
        on = [(xx + 1, yy) for yy in range(7) for xx in range(5) if g[yy][xx] == '1']
        ox = x + i * 8
        for (gx, gy) in on:
            for dx in (-1, 0, 1):
                for dy in (-1, 0, 1):
                    img.putpixel((ox + gx + dx, y + gy + dy), ol)
        for (gx, gy) in on:
            img.putpixel((ox + gx, y + gy), fg)

HUD_BG = BLUE

def gen_font():
    data = []
    for code in range(32, 91):
        data += tile_bytes(glyph_pixels(chr(code), WHITE, HUD_BG))
    return data

# ------------------------------------------------------------------ game screen frame + HUD
VIEW_X, VIEW_Y = 4, 3            # tile coords of the 24x12 view
FACE_TX, FACE_TY = 14, 17

def gen_frame():
    img = Image.new('L', (256, 192), BLUE)
    d = ImageDraw.Draw(img)
    # polka dots (cycling colour) on the side panels and HUD
    for ty in range(24):
        for tx in range(32):
            if (tx + ty) % 2 == 0:
                cx, cy = tx * 8 + 4, ty * 8 + 4
                d.ellipse([cx - 2, cy - 2, cx + 1, cy + 1], fill=CYC_A)
    # top bar (text area) plain
    d.rectangle([0, 0, 255, 15], fill=HUD_BG)
    d.rectangle([0, 16, 255, 17], fill=BLACK)
    # view frame: thick black + yellow bevel
    vx0, vy0, vx1, vy1 = VIEW_X * 8, VIEW_Y * 8, VIEW_X * 8 + 191, VIEW_Y * 8 + 95
    d.rectangle([vx0 - 5, vy0 - 5, vx1 + 5, vy1 + 5], fill=BLACK)
    d.rectangle([vx0 - 4, vy0 - 4, vx1 + 4, vy1 + 4], fill=YELLOW)
    d.rectangle([vx0 - 2, vy0 - 2, vx1 + 2, vy1 + 2], fill=BLACK)
    # bottom HUD panel
    d.rectangle([0, 124, 255, 191], fill=HUD_BG)
    d.rectangle([0, 124, 255, 125], fill=BLACK)
    for (x0, x1) in ((8, 87), (168, 247)):
        d.rounded_rectangle([x0, 130, x1, 185], radius=6, fill=BLACK)
        d.rounded_rectangle([x0 + 2, 132, x1 - 2, 183], radius=5, fill=DKRED if x0 < 100 else DKGREEN)
        d.rectangle([x0 + 8, 160, x1 - 8, 175], fill=HUD_BG)     # number well
    # face frame
    fx, fy = FACE_TX * 8, FACE_TY * 8
    d.rounded_rectangle([fx - 6, fy - 6, fx + 37, fy + 37], radius=6, fill=BLACK)
    d.rounded_rectangle([fx - 4, fy - 4, fx + 35, fy + 35], radius=5, fill=YELLOW)
    d.rectangle([fx, fy, fx + 31, fy + 31], fill=BLUE)
    draw_text_img(img, 24, 136, 'HEALTH', YELLOW)
    draw_text_img(img, 192, 136, 'AMMO', YELLOW)
    draw_text_img(img, 104, 177, "SPLAT!", WHITE)
    # icons
    a = Art(16, 16); draw_heart(a, 0); ic = a.result()
    for y in range(16):
        for x in range(16):
            if ic[y][x] != T: img.putpixel((64 + x, 140 + y), ic[y][x])
    a = Art(16, 16); draw_ammo(a, 0); ic = a.result()
    for y in range(16):
        for x in range(16):
            if ic[y][x] != T: img.putpixel((228 + x, 140 + y), ic[y][x])
    # cut into tiles (view and face cells excluded)
    px = img.load()
    tiles, index, tmap = [], {}, []
    for ty in range(24):
        for tx in range(32):
            inview = VIEW_X <= tx < VIEW_X + 24 and VIEW_Y <= ty < VIEW_Y + 12
            inface = FACE_TX <= tx < FACE_TX + 4 and FACE_TY <= ty < FACE_TY + 4
            if inview or inface:
                tmap.append(0xFFFF)
                continue
            blk = tuple(tuple(px[tx * 8 + x, ty * 8 + y] for x in range(8)) for y in range(8))
            for row in blk:
                assert 0 not in row
            if blk not in index:
                index[blk] = len(tiles)
                tiles.append(blk)
            tmap.append(index[blk])
    return tiles, tmap, img

def gen_faces():
    faces = []
    def face(kind):
        a = Art(32, 32, ss=4)
        # helmet + face (yellow cartoon skin)
        a.ell(0.5, 0.58, 0.36, 0.36, YELLOW)
        a.poly([(0.12, 0.42), (0.18, 0.12), (0.5, 0.04), (0.82, 0.12), (0.88, 0.42)], GREEN)
        a.rect(0.1, 0.36, 0.9, 0.44, DKGREEN)
        a.ell(0.5, 0.2, 0.06, 0.05, CYC_A)
        ey = 0.56
        if kind == 4:
            for ex in (0.36, 0.64):
                a.line([(ex - 0.07, ey - 0.07), (ex + 0.07, ey + 0.07)], BLACK, 1.0)
                a.line([(ex - 0.07, ey + 0.07), (ex + 0.07, ey - 0.07)], BLACK, 1.0)
            a.ell(0.5, 0.8, 0.08, 0.04, BLACK, ol=False)
            return a.result()
        if kind == 5:
            a.line([(0.28, 0.52), (0.42, 0.58)], BLACK, 1.2)
            a.line([(0.72, 0.52), (0.58, 0.58)], BLACK, 1.2)
            a.ell(0.5, 0.8, 0.1, 0.08, DKRED)
            return a.result()
        for ex in (0.37, 0.63):
            a.ell(ex, ey, 0.1, 0.11, WHITE)
            dx = 0.0 if kind < 2 else (0.03 if ex < 0.5 else -0.03)
            a.ell(ex + dx, ey + 0.02, 0.04, 0.05, BLACK, ol=False)
        if kind == 0:
            a.poly([(0.3, 0.72), (0.7, 0.72), (0.5, 0.88)], WHITE)
        elif kind == 1:
            a.line([(0.34, 0.76), (0.42, 0.81), (0.58, 0.81), (0.66, 0.76)], BLACK, 1.0)
        elif kind == 2:
            a.line([(0.36, 0.8), (0.64, 0.8)], BLACK, 1.0)
            a.ell(0.8, 0.5, 0.04, 0.06, BLUE, ol=False)     # sweat
        elif kind == 3:
            a.line([(0.34, 0.82), (0.42, 0.77), (0.58, 0.77), (0.66, 0.82)], BLACK, 1.0)
            a.rect(0.6, 0.62, 0.84, 0.7, WHITE)             # bandage
        return a.result()
    data = []
    for k in range(6):
        img = face(k)
        for ty in range(4):
            for tx in range(4):
                blk = [[(BLUE if img[ty * 8 + y][tx * 8 + x] == T else img[ty * 8 + y][tx * 8 + x])
                        for x in range(8)] for y in range(8)]
                data += tile_bytes(blk)
    return data

def gen_weapon():
    """paint blaster held in the right hand, nozzle aimed at the view centre;
    2 frames (idle, firing) x 3x2 tall sprites, + crosshair (1 tall sprite)"""
    out = []
    for fr in range(2):
        a = Art(24, 32, ss=4)
        r = 0.05 if fr else 0.0
        # barrel (diagonal) from the grip up-left to the nozzle
        a.poly([(0.62 + r, 0.98), (0.98 + r, 0.86), (0.42 + r, 0.2 + r), (0.16 + r, 0.3 + r)], RED)
        a.poly([(0.7 + r, 0.86), (0.84 + r, 0.8), (0.44 + r, 0.34 + r), (0.34 + r, 0.38 + r)], YELLOW, ol=False)
        # nozzle
        a.poly([(0.12 + r, 0.3 + r), (0.42 + r, 0.18 + r), (0.36 + r, 0.04 + r), (0.04 + r, 0.16 + r)], BLUE)
        a.ell(0.2 + r, 0.12 + r, 0.12, 0.06, BLACK, ol=False)
        # paint tank
        a.ell(0.7 + r, 0.42 + r, 0.24, 0.17, WHITE)
        a.ell(0.7 + r, 0.46 + r, 0.2, 0.11, CYC_A, ol=False)
        a.ell(0.62 + r, 0.36 + r, 0.06, 0.03, WHITE, ol=False)
        # white cartoon glove on the grip
        a.ell(0.78 + r, 0.88, 0.22, 0.13, WHITE)
        a.line([(0.66 + r, 0.8), (0.66 + r, 0.97)], BLACK, 0.7)
        a.line([(0.8 + r, 0.79), (0.8 + r, 0.98)], BLACK, 0.7)
        if fr:
            a.ell(0.16, 0.12, 0.17, 0.12, CYC_B)
            a.ell(0.16, 0.12, 0.08, 0.06, WHITE, ol=False)
        img = a.result()
        for c in range(3):
            for rr in range(2):
                blk = [[(0 if img[rr * 16 + y][c * 8 + x] == T else img[rr * 16 + y][c * 8 + x])
                        for x in range(8)] for y in range(16)]
                out += tile_bytes(blk[:8]) + tile_bytes(blk[8:])
    # crosshair: centred on pixel (3,7) of a tall sprite -> placed at (125, 65)
    blk = [[0] * 8 for _ in range(16)]
    for d in (-3, -2, 2, 3):
        blk[7 + d][3] = WHITE
        blk[7][3 + d] = WHITE
    for y in range(16):
        for x in range(8):
            if blk[y][x] == WHITE:
                for dx in (-1, 0, 1):
                    for dy in (-1, 0, 1):
                        xx, yy = x + dx, y + dy
                        if 0 <= xx < 8 and 0 <= yy < 16 and blk[yy][xx] == 0:
                            blk[yy][xx] = BLACK
    out += tile_bytes(blk[:8]) + tile_bytes(blk[8:])
    return out

# ------------------------------------------------------------------ title screen
TITLE_PAL = [(0, 0, 0), (3, 3, 3), (3, 3, 0), (3, 2, 0), (3, 0, 0), (1, 3, 0), (0, 1, 3), (3, 3, 3),
             (3, 0, 0), (3, 2, 0), (3, 3, 0), (1, 3, 0), (0, 3, 3), (0, 1, 3), (2, 0, 3), (3, 0, 3)]

def gen_title():
    img = Image.new('L', (256, 192), 0)
    px = img.load()
    for y in range(192):
        for x in range(256):
            px[x, y] = 8 + (((x + y) // 8) % 8)
    d = ImageDraw.Draw(img)
    d.fontmode = '1'
    fpath = '/usr/share/fonts/truetype/google-fonts/Poppins-Bold.ttf'
    big = ImageFont.truetype(fpath, 54)
    mid = ImageFont.truetype(fpath, 30)
    # cartoon splat behind the logo
    for (cx, cy, r) in ((128, 66, 62), (60, 40, 16), (200, 34, 14), (210, 98, 18), (48, 100, 12)):
        d.ellipse([cx - r - 3, cy - r * 0.8 - 3, cx + r + 3, cy + r * 0.8 + 3], fill=0)
    for (cx, cy, r) in ((128, 66, 62), (60, 40, 16), (200, 34, 14), (210, 98, 18), (48, 100, 12)):
        d.ellipse([cx - r, cy - r * 0.8, cx + r, cy + r * 0.8], fill=1)
    d.text((128 + 3, 52 + 3), 'SPLAT', font=big, fill=0, anchor='mm', stroke_width=3, stroke_fill=0)
    d.text((128, 52), 'SPLAT', font=big, fill=2, anchor='mm', stroke_width=3, stroke_fill=0)
    d.text((128 + 2, 94 + 2), 'TROOPER', font=mid, fill=0, anchor='mm', stroke_width=2, stroke_fill=0)
    d.text((128, 94), 'TROOPER', font=mid, fill=4, anchor='mm', stroke_width=2, stroke_fill=0)
    # 3D badge
    d.ellipse([196, 104, 236, 140], fill=0)
    d.ellipse([198, 106, 234, 138], fill=3)
    d.text((216, 122), '3D', font=ImageFont.truetype(fpath, 18), fill=1, anchor='mm', stroke_width=1, stroke_fill=0)
    # a blob mascot bottom-left
    a = Art(40, 40, ss=4); draw_blob(a, 2); bl = a.result()
    tmap = {GREEN: 5, DKGREEN: 5, WHITE: 1, BLACK: 0, DKRED: 4, RED: 4, CYC_B: 12, YELLOW: 2}
    for y in range(40):
        for x in range(40):
            if bl[y][x] != T:
                px[18 + x, 112 + y] = tmap.get(bl[y][x], 1)
    a = Art(40, 40, ss=4); draw_imp(a, 0); bl = a.result()
    tmap2 = {RED: 4, DKRED: 4, YELLOW: 2, WHITE: 1, BLACK: 0, ORANGE: 3, GREY: 7}
    for y in range(40):
        for x in range(40):
            if bl[y][x] != T:
                px[150 + x, 112 + y] = tmap2.get(bl[y][x], 1)
    d.rectangle([40, 158, 215, 175], fill=0)
    draw_text_img(img, 68, 163, 'PRESS BUTTON 1', 7, 0)
    draw_text_img(img, 36, 180, '2026 HOMEBREW - SMS', 1, 0)
    tiles, index, tm = [], {}, []
    for ty in range(24):
        for tx in range(32):
            blk = tuple(tuple(px[tx * 8 + x, ty * 8 + y] for x in range(8)) for y in range(8))
            if blk not in index:
                index[blk] = len(tiles)
                tiles.append(blk)
            tm.append(index[blk])
    assert len(tiles) <= 440, len(tiles)
    return tiles, tm, img

# ------------------------------------------------------------------ levels
LEVELS = [
("TOONTOWN BACKYARD", [
"################################",
"#P.....#........#.......#......#",
"#......#....*...#...b...#..*...#",
"#..a...........................#",
"#......#....b...#.......#...i..#",
"###..###........###..####......#",
"#..........................a...#",
"#..*....HHHH.......HHH......b..#",
"#.......H..H...b...H*H.........#",
"#...b...H..H.......H.H.....HHHH#",
"#.......HHHH.......H.H.....H..E#",
"###..####......a...........H...#",
"#.........WWWWWW..WWWW.....HH..#",
"#..i......W....W.....W.........#",
"#.........W.*..W.....W...i...h.#",
"#....h....W.b........W.........#",
"#.........W....W..i............#",
"################################",
]),
("CANDY FACTORY", [
"################################",
"#P....C.......CCCC.......C....*#",
"#.....C..i....C..C...b...C.....#",
"#..a..C.......C..C.......C..i..#",
"#.....CCCC..CCC..CCC..CCCC.....#",
"#..........h.........a.........#",
"WWWW..WWWW...SSSSSS....WWWW..WWW",
"#.......W....S....S....W.......#",
"#..b....W..i.S.*..S..i.W...b...#",
"#.......W....S....S....W.......#",
"#..*.........SS..SS............#",
"#.......i...............r......#",
"CCCCC..CCCCCCCC..CCCCCCCCC..CCCC",
"#.......C...............C......#",
"#..r....C..a....i....h..C...*..#",
"#.......C...............C..r...#",
"#..h.................b......E..#",
"################################",
]),
("DR. GLOOP'S LAB", [
"SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS",
"SP.....S......r.......S.......*S",
"S......S..............S..i.....S",
"S..a...H....HHHHHH....H........S",
"S......S....H....H....S...r....S",
"SSSS.SSS....H.*..H....SSSS..SSSS",
"S...........H....H.............S",
"S..r.........H..H.......b...a..S",
"S......b.......h.........r.....S",
"SCCCCC..CCCC......CCCC..CCCCC..S",
"S....C..C............C..C......S",
"S.h..C..C..i....i....C..C..r...S",
"S....C..C....S..S....C..C......S",
"S...........SS..SS.............S",
"S..*.....r..S....S..r.....a....S",
"S...........S.B..S.............S",
"S..i...a....SS..SS.....i.....E.S",
"SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS",
]),
]
WALLS = {'#': 1, 'W': 2, 'H': 3, 'S': 4, 'C': 5, 'E': 6}
OBJS = {'b': 0, 'i': 1, 'r': 2, 'B': 3, 'h': 4, 'a': 5, '*': 6}

def gen_levels():
    out = []
    for name, rows in LEVELS:
        assert len(rows) <= 32
        cells = [1] * 1024
        objs = []
        start = None
        for y, row in enumerate(rows):
            assert len(row) == 32, (name, y, len(row))
            for x, ch in enumerate(row):
                if ch in WALLS:
                    cells[y * 32 + x] = WALLS[ch]
                else:
                    cells[y * 32 + x] = 0
                    if ch == 'P':
                        start = (x, y)
                    elif ch in OBJS:
                        objs.append((OBJS[ch], x, y))
        for y in range(len(rows), 32):
            for x in range(32):
                cells[y * 32 + x] = 1
        assert start
        # flood fill: every object and the exit portal must be reachable
        seen, stack = set(), [start]
        while stack:
            x, y = stack.pop()
            if (x, y) in seen or cells[y * 32 + x]:
                continue
            seen.add((x, y))
            stack += [(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)]
        for (t, x, y) in objs:
            assert (x, y) in seen, (name, 'unreachable object', t, x, y)
        ex = [(x, y) for y in range(32) for x in range(32) if cells[y * 32 + x] == 6]
        assert any((x + dx, y + dy) in seen for (x, y) in ex for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))), (name, 'exit unreachable')
        out.append((name, cells, objs, start))
    return out

# ------------------------------------------------------------------ music
NOTE_NAMES = {'C': 0, 'C#': 1, 'D': 2, 'D#': 3, 'E': 4, 'F': 5, 'F#': 6, 'G': 7, 'G#': 8, 'A': 9, 'A#': 10, 'B': 11}

def period(note):
    if note == 'R':
        return 0
    n, o = note[:-1], int(note[-1])
    midi = 12 * (o + 1) + NOTE_NAMES[n]
    f = 440.0 * 2 ** ((midi - 69) / 12.0)
    return max(1, min(1023, int(round(3579545 / (32 * f)))))

def song(text, unit):
    data = []
    for tok in text.split():
        nt, ln = tok.split(':')
        p = period(nt)
        dur = int(ln) * unit
        data += [p & 0xFF, (p >> 8) | (0x80 if nt == 'R' else 0), dur]
    return data + [0, 0x40, 0]          # 0x40 flag: loop/end marker

SONGS = {
 'title': (
  "C5:2 E5:2 G5:2 C6:2 B5:2 G5:2 E5:4 F5:2 A5:2 C6:2 A5:2 G5:6 R:2 "
  "E5:2 G5:2 A5:2 G5:2 E5:2 C5:2 D5:4 E5:2 D5:2 C5:2 D5:2 C5:6 R:2",
  "C3:4 G3:4 C3:4 G3:4 F3:4 C4:4 G3:4 G3:4 A3:4 E3:4 F3:4 C3:4 G3:4 G3:4 C3:4 C3:4", 7),
 'game': (
  "E5:1 R:1 E5:1 G5:1 R:1 A5:1 G5:2 E5:1 R:1 D5:1 C5:1 D5:2 R:2 "
  "E5:1 R:1 E5:1 G5:1 R:1 A5:1 C6:2 B5:1 A5:1 G5:1 E5:1 G5:2 R:2 "
  "A5:1 R:1 A5:1 G5:1 E5:1 G5:1 A5:2 G5:1 E5:1 D5:1 C5:1 D5:2 R:2 "
  "E5:1 D5:1 C5:1 D5:1 E5:1 G5:1 E5:2 D5:1 R:1 C5:1 R:1 C5:2 R:2",
  "C3:2 G3:2 C3:2 G3:2 C3:2 G3:2 F3:2 G3:2 C3:2 G3:2 C3:2 G3:2 A3:2 G3:2 F3:2 G3:2 "
  "F3:2 C4:2 F3:2 C4:2 C3:2 G3:2 C3:2 G3:2 G2:2 D3:2 G2:2 D3:2 C3:2 G3:2 C3:4", 8),
 'clear': ("C5:1 E5:1 G5:1 C6:3 G5:1 C6:6 R:8", "C3:3 G3:3 C4:6 R:8", 6),
 'over': ("G4:3 F#4:3 F4:3 E4:8 R:8", "C3:3 B2:3 A#2:3 A2:8 R:8", 9),
}

def gen_music():
    s = ''
    for k, (a, b, unit) in SONGS.items():
        s += c_array('mus_%s_a' % k, song(a, unit))
        s += c_array('mus_%s_b' % k, song(b, unit))
    return s

# ------------------------------------------------------------------ main
def main():
    s_tab = gen_tables()
    write('tables.c', s_tab)
    write('scalers.s', gen_unrolled_asm())
    banks = gen_texbanks()
    TEXBANK0 = 2
    for i, b in enumerate(banks):
        write('texbank%d.c' % i, c_array('texbank%d' % i, list(b), 'unsigned char', 64))
    ANG_BANK = TEXBANK0 + len(banks)
    write('angbank.c', c_array('angbank', list(gen_angbank()), 'unsigned char', 64))
    write('assets.inc', 'TEXBANK0 = %d\nANG_BANK = %d\nROWS_BASE = 0x%04X\n' % (TEXBANK0, ANG_BANK, 0x8000 + 0x26 * 256))
    # misc bank: levels, faces, weapon, frame tiles
    MISC_BANK = ANG_BANK + 1
    lv = gen_levels()
    misc = '#include "../src/defs.h"\n'
    for i, (name, cells, objs, start) in enumerate(lv):
        misc += c_array('lvl%d_map' % i, cells, 'unsigned char', 32)
        flat = []
        for (t, x, y) in objs:
            flat += [t, x, y]
        flat += [0xFF]
        misc += c_array('lvl%d_objs' % i, flat)
    misc += c_array('face_tiles', gen_faces())
    misc += c_array('weapon_tiles', gen_weapon())
    write('miscbank.c', misc)
    tt, tm, timg = gen_title()
    tb = '#include "../src/defs.h"\n'
    tb += c_array('title_tiles', [b for t in tt for b in tile_bytes(t)])
    tb += c_array('title_map', tm, 'unsigned short', 32)
    write('titlebank.c', tb)
    ftiles, fmap, fimg = gen_frame()
    TITLE_BANK = MISC_BANK + 1
    SPR_BANK = MISC_BANK + 2
    fixed, srcs, preview = gen_sprites(SPR_BANK)
    for i, src in enumerate(srcs):
        write('sprbank%d.c' % i, '#include "spr_bank%d.h"\n' % i + src)
    write('sprites_tab.c', fixed)
    # fixed-bank assets
    fx = '#include "../src/defs.h"\n'
    fx += c_array('font_tiles', gen_font())
    fx += c_array('frame_tiles', [b for t in ftiles for b in tile_bytes(t)])
    fx += c_array('frame_map', [0xFF if v == 0xFFFF else v for v in fmap], 'unsigned char', 32)
    fx += gen_music()
    pal_bytes = []
    for L in range(3):
        for which in range(2):
            pal_bytes += [smsc(*c) for c in full_palette(L, which)]
    fx += c_array('level_pals', pal_bytes)
    fx += c_array('cycle_cols', [smsc(*c) for c in CYCLE])
    fx += c_array('title_pal', [smsc(*c) for c in TITLE_PAL])
    names = ''.join('"%s",' % n for (n, _, _, _) in lv)
    fx += 'const char * const level_names[%d] = {%s};\n' % (len(lv), names)
    starts = []
    for (_, _, _, st) in lv:
        starts += [st[0], st[1]]
    fx += c_array('level_starts', starts)
    write('assets_fixed.c', fx)
    # header with bank numbers & sizes
    h = '/* generated by tools/gen.py */\n#ifndef ASSETS_H\n#define ASSETS_H\n'
    h += '#define TITLE_BANK %d\n#define ANG_BANK %d\n' % (TITLE_BANK, ANG_BANK)
    h += '#define TEXBANK0 %d\n#define NTEXBANKS %d\n#define MISC_BANK %d\n#define SPR_BANK0 %d\n#define NSPRBANKS %d\n' % (
        TEXBANK0, len(banks), MISC_BANK, SPR_BANK, len(srcs))
    h += '#define NFRAME_TILES %d\n#define NTITLE_TILES %d\n#define NLEVELS %d\n#define NSIZES %d\n#define NGRAPHICS %d\n' % (
        len(ftiles), len(tt), len(lv), len(SIZES), len(GRAPHICS))
    h += '#define ROWS_BASE 0x%04X\n#define NK_ENTRIES %d\n' % (0x8000 + 0x26 * 256, NK)
    for i, (nm, _, _) in enumerate(GRAPHICS):
        h += '#define G_%s %d\n' % (nm.upper(), i)
    h += 'static const unsigned char size_px[%d] = {%s};\n' % (len(SIZES), ','.join(map(str, SIZES)))
    h += '#endif\n'
    write('assets.h', h)
    # bank list for the Makefile
    bl = ['TEXBANKS = ' + ' '.join(str(TEXBANK0 + i) for i in range(len(banks))), 'ANG_BANK = %d' % ANG_BANK,
          'MISC_BANK = %d' % MISC_BANK, 'TITLE_BANK = %d' % TITLE_BANK,
          'SPRBANKS = ' + ' '.join(str(SPR_BANK + i) for i in range(len(srcs)))]
    write('banks.mk', '\n'.join(bl) + '\n')
    # previews
    pv = os.path.join(HERE, '..', 'shots')
    os.makedirs(pv, exist_ok=True)
    preview.resize((preview.width * 3, preview.height * 3), Image.NEAREST).save(os.path.join(pv, 'sprites_preview.png'))
    pal0 = [rgb8(c) for c in full_palette(0, 0)]
    def colorize(im, pal):
        o = Image.new('RGB', im.size)
        o.putdata([pal[v] if v < len(pal) else (255, 0, 255) for v in im.getdata()])
        return o
    colorize(fimg, pal0).resize((768, 576), Image.NEAREST).save(os.path.join(pv, 'frame_preview.png'))
    colorize(timg, [rgb8(c) for c in TITLE_PAL]).resize((768, 576), Image.NEAREST).save(os.path.join(pv, 'title_preview.png'))
    vs = variants()
    tp = Image.new('RGB', (34 * len(vs), 32))
    for i, t in enumerate(vs):
        full = t + t[::-1]
        for y in range(32):
            for x in range(32):
                tp.putpixel((i * 34 + x, y), rgb8(full_palette(0, 0)[full[y][x]]))
    tp.resize((tp.width * 4, 128), Image.NEAREST).save(os.path.join(pv, 'textures_preview.png'))
    print('frame tiles', len(ftiles), 'title tiles', len(tt), 'tex banks', len(banks), 'spr banks', len(srcs))

if __name__ == '__main__':
    main()
