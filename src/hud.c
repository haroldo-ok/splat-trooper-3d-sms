/* hud.c - text, numbers, the trooper's face */
#include <SMSlib.h>
#include "defs.h"

static u8 cur_face = 0xFF;

void print_at(u8 x, u8 y, const char *s)
{
    SMS_setNextTileatXY(x, y);
    while (*s) {
        u8 c = (u8)*s++;
        if (c < 32 || c > 90) c = 32;
        SMS_setTile((FONT_BASE + c - 32) | TILE_PRIORITY);
    }
}

/* digits: number of characters; | 0x80: blank leading zeros */
void print_num(u8 x, u8 y, u16 v, u8 digits)
{
    char buf[7];
    u8 blank = digits & 0x80;
    u8 i;
    digits &= 0x7F;
    i = digits;
    buf[digits] = 0;
    while (i) {
        i--;
        if (blank && v == 0 && i != digits - 1) buf[i] = ' ';
        else buf[i] = '0' + (u8)(v % 10);
        v /= 10;
    }
    print_at(x, y, buf);
}

static void hud_labels(void)
{
    print_at(1, 1, "SCORE");
    print_at(21, 1, "LEVEL");
    print_num(27, 1, level + 1, 1);
}

static void face_update(void)
{
    u8 f;
    if (health == 0) f = 4;
    else if (face_ouch) f = 5;
    else if (health > 75) f = 0;
    else if (health > 50) f = 1;
    else if (health > 25) f = 2;
    else f = 3;
    if (f != cur_face) {
        cur_face = f;
        SMS_mapROMBank(MISC_BANK);
        SMS_loadTiles(face_tiles + (u16)f * 512, 256 + SAT_FACE, 512);
    }
}

void hud_draw(void)
{
    u8 d = hud_dirty;
    if (face_ouch && cur_face != 5) d |= 8;
    if (!d) return;
    hud_dirty = 0;
    if (d & 16) { print_at(0, 1, " SPLAT DR. GLOOP'S MEGA-BOT!   "); d &= ~4; }
    if (d & 32) { print_at(0, 1, "                                "); hud_labels(); d |= 4; }
    if (msg_timer) d &= ~4;
    if (d & 1) print_num(4, 20, health, 0x83);
    if (d & 2) print_num(24, 20, ammo, 0x82);
    if (d & 4) print_num(7, 1, score, 5);
    if (d & 9) face_update();
}

void hud_full(void)
{
    u8 x, y;
    cur_face = 0xFF;
    hud_labels();
    /* the face cells point at the 16 face tiles in the sprite area */
    for (y = 0; y < 4; y++) {
        SMS_setNextTileatXY(14, 17 + y);
        for (x = 0; x < 4; x++)
            SMS_setTile((256 + SAT_FACE + y * 4 + x) | TILE_PRIORITY);
    }
    hud_dirty = 0x0F;
    hud_draw();
}
