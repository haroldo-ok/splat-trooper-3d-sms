/* sound.c - tiny PSG driver, run from the frame interrupt.
 * Music on tone channels 0/1, sound effects on channel 2 + noise. */
#include <SMSlib.h>
#include "defs.h"

__sfr __at 0x7F PSGPort;
__sfr __at 0xDC IOPortA;

volatile u8 vbl_count;
volatile u8 pad_held, pad_press;
static u8 pad_prev;

static const u8 *mus_p[2], *mus_start[2];
static u8 mus_t[2], mus_el[2], mus_rest[2];
static u8 mus_loop, mus_on;
static const u8 mus_base[2] = { 2, 5 };
static volatile u8 sfx_id, sfx_t;

static void psg_tone(u8 ch, u16 per)
{
    PSGPort = 0x80 | (ch << 5) | (per & 0x0F);
    PSGPort = (per >> 4) & 0x3F;
}

static void psg_vol(u8 ch, u8 att)
{
    PSGPort = 0x90 | (ch << 5) | (att > 15 ? 15 : att);
}

void snd_init(void)
{
    u8 c;
    mus_on = 0;
    sfx_id = 0;
    for (c = 0; c < 4; c++) psg_vol(c, 15);
}

void music_play(const u8 *a, const u8 *b, u8 loop)
{
    __asm__("di");
    mus_start[0] = mus_p[0] = a;
    mus_start[1] = mus_p[1] = b;
    mus_t[0] = mus_t[1] = 1;
    mus_loop = loop;
    mus_on = 3;
    __asm__("ei");
}

void music_stop(void)
{
    mus_on = 0;
    psg_vol(0, 15);
    psg_vol(1, 15);
}

u8 music_busy(void) { return mus_on; }

void sfx(u8 id)
{
    sfx_id = id;
    sfx_t = 0;
}

static void music_step(u8 c)
{
    const u8 *p;
    if (!(mus_on & (1 << c))) return;
    if (--mus_t[c]) {
        mus_el[c]++;
        if (!mus_rest[c]) psg_vol(c, mus_base[c] + (mus_el[c] >> 3));
        return;
    }
    p = mus_p[c];
    if (p[1] & 0x40) {
        if (!mus_loop) { mus_on &= ~(1 << c); psg_vol(c, 15); return; }
        p = mus_start[c];
    }
    mus_t[c] = p[2];
    mus_el[c] = 0;
    if (p[1] & 0x80) {
        mus_rest[c] = 1;
        psg_vol(c, 15);
    } else {
        mus_rest[c] = 0;
        psg_tone(c, p[0] | ((u16)(p[1] & 3) << 8));
        psg_vol(c, mus_base[c]);
    }
    mus_p[c] = p + 3;
}

static const u16 arp[] = { 254, 214, 170, 127, 107, 85 };

static void sfx_step(void)
{
    u8 t = sfx_t, len;
    switch (sfx_id) {
    case 0: return;
    case SFX_SHOOT:
        len = 10;
        psg_tone(2, 60 + t * 28); psg_vol(2, t);
        PSGPort = 0xE4; psg_vol(3, 3 + t);
        break;
    case SFX_HIT:
        len = 9;
        psg_tone(2, 340 - t * 32); psg_vol(2, 1 + (t >> 1));
        break;
    case SFX_PICK:
        len = 12;
        psg_tone(2, arp[t >> 2]); psg_vol(2, 1 + (t & 3));
        break;
    case SFX_STAR:
        len = 12;
        psg_tone(2, arp[2 + (t >> 2)] >> 1); psg_vol(2, 1 + (t & 3));
        break;
    case SFX_HURT:
        len = 14;
        psg_tone(2, 700 + t * 12); psg_vol(2, 3 + (t >> 1));
        PSGPort = 0xE6; psg_vol(3, 2 + (t >> 1));
        break;
    case SFX_DIE:
        len = 22;
        psg_tone(2, 120 + t * 24 + ((t & 2) ? 20 : 0)); psg_vol(2, t >> 1);
        break;
    case SFX_ESHOT:
        len = 8;
        PSGPort = 0xE5; psg_vol(3, 4 + t);
        break;
    case SFX_EXIT:
        len = 24;
        psg_tone(2, arp[(t >> 2) % 6] >> (t >> 4)); psg_vol(2, 1 + (t & 3));
        break;
    default: len = 0;
    }
    if (++sfx_t >= len) {
        sfx_id = 0;
        psg_vol(2, 15);
        psg_vol(3, 15);
    }
}

/* frame interrupt: count frames, latch the pad, run the sound */
void snd_isr(void)
{
    u8 k = ~IOPortA & 0x3F;
    vbl_count++;
    pad_press |= k & ~pad_prev;
    pad_held = k;
    pad_prev = k;
    music_step(0);
    music_step(1);
    sfx_step();
}
