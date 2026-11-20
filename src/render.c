/* render.c - ray casting and the mirrored-half view.
 *
 * Name-table layout of the view (24 x 12 tiles at VIEW_TX,VIEW_TY):
 *   top row r (0..5)        -> tile VIEW_BASE + tc*6 + r   (BG palette: sky)
 *   bottom row 11-r         -> same tile | FLIP_Y | SPRITE_PALETTE (floor)
 * A cell that is pure sky in the top half points at SKY_BASE + r instead (and
 * its mirror shows the floor gradient), so it costs no tile upload at all.
 */
#include <SMSlib.h>
#include "defs.h"

u16 zbuf[NRAYS];
u16 center_dist;
extern u8 sig[NRAYS * 4];
extern u8 nt_state[VIEW_TW * 6];
void ray_init(void);
void qsq_init(void);

#define TILE_FLAGS_BOTTOM (TILE_FLIPPED_Y | TILE_USE_SPRITE_PALETTE)

void render_invalidate(void)
{
    u8 i;
    for (i = 0; i < NRAYS * 4; i++)
        sig[i] = 0xFF;
    for (i = 0; i < VIEW_TW * 6; i++)
        nt_state[i] = 0xFF;
}

void render_init(void)
{
    u8 *d = LUT;
    const u8 *s = plane_lut;
    u16 n;
    for (n = 0; n < 1024; n++)
        *d++ = *s++;
    ray_init();
    qsq_init();
    SMS_loadTiles(sky_tiles, SKY_BASE, 192);
    render_invalidate();
}
