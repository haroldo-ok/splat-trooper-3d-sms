/* sprites.c - objects drawn with PRE-SCALED hardware sprites.
 *
 * Every object graphic exists in ROM at 8 sizes (8..48 px, made offline by
 * tools/gen.py).  Each frame an object is projected, the nearest pre-scaled
 * size is picked, and the image is placed with 8x16 hardware sprites.  The
 * tiles of the images in use live in a small VRAM cache (SAT tiles 80..191),
 * uploaded only when an image first appears.  Walls hide sprites per 8 px
 * column using the ray depth buffer; the HUD/frame tiles have the priority
 * bit set so sprites never spill outside the view.
 */
#include <SMSlib.h>
#include "defs.h"

u8 weapon_frame;
s8 weapon_bob;

/* per object type: graphic base, world height and lift (1/128 tile) */
static const u8 type_gfx[]    = { G_BLOB_WALK0, G_IMP_WALK0, G_ROBOT_WALK0, G_ROBOT_WALK0,
                                  G_HEART, G_AMMO, G_STAR, G_FIREBALL, G_PLASMA, G_SPLAT };
static const u8 type_height[] = { 74, 92, 104, 150, 44, 44, 50, 40, 40, 44 };
static const u8 type_z[]      = { 0, 0, 0, 0, 0, 0, 10, 36, 36, 40 };

#define NCE 16
static u8 c_g[NCE], c_s[NCE], c_pair[NCE], c_np[NCE], c_last[NCE];
static u8 pair_own[CACHE_PAIRS];
static u8 upload_budget;

void sprites_level_init(void)
{
    u8 i;
    for (i = 0; i < NCE; i++) c_g[i] = 0xFF;
    for (i = 0; i < CACHE_PAIRS; i++) pair_own[i] = 0xFF;
    SMS_mapROMBank(MISC_BANK);
    SMS_loadTiles(weapon_tiles, 256 + SAT_WEAPON, 26 * 32);
}

static u8 find_run(u8 np)
{
    u8 i, run = 0;
    for (i = 0; i < CACHE_PAIRS; i++) {
        if (pair_own[i] == 0xFF) {
            run++;
            if (run == np) return i + 1 - np;
        } else run = 0;
    }
    return 0xFF;
}

static void evict(u8 e)
{
    u8 i;
    for (i = 0; i < c_np[e]; i++) pair_own[c_pair[e] + i] = 0xFF;
    c_g[e] = 0xFF;
}

static u8 evict_oldest(void)
{
    u8 e, best = 0xFF, age = 0;
    for (e = 0; e < NCE; e++) {
        if (c_g[e] != 0xFF && c_last[e] != frame_no) {
            u8 a = frame_no - c_last[e];
            if (a >= age) { age = a; best = e; }
        }
    }
    if (best == 0xFF) return 0;
    evict(best);
    return 1;
}

/* returns the SAT tile of image (g,s); *ps may be changed to a cached
   neighbour size when the upload budget for this frame is spent */
static u8 cache_get(u8 g, u8 *ps)
{
    u8 e, s = *ps, np, pair, best = 0xFF, bd = 0xFF;
    const SprImg *img;
    for (e = 0; e < NCE; e++) {
        if (c_g[e] == g) {
            u8 d;
            if (c_s[e] == s) { c_last[e] = frame_no; return SAT_CACHE + c_pair[e] * 2; }
            d = c_s[e] > s ? c_s[e] - s : s - c_s[e];
            if (d < bd) { bd = d; best = e; }
        }
    }
    img = &spr_imgs[g * NSIZES + s];
    np = img->npieces;
    if (np > upload_budget) {
        if (best != 0xFF) {
            *ps = c_s[best];
            c_last[best] = frame_no;
            return SAT_CACHE + c_pair[best] * 2;
        }
        return 0xFF;
    }
    for (;;) {
        pair = find_run(np);
        if (pair != 0xFF) break;
        if (!evict_oldest()) return 0xFF;
    }
    for (e = 0; e < NCE; e++)
        if (c_g[e] == 0xFF) break;
    if (e == NCE) {
        if (!evict_oldest()) return 0xFF;
        for (e = 0; e < NCE; e++)
            if (c_g[e] == 0xFF) break;
        pair = find_run(np);
        if (pair == 0xFF) return 0xFF;
    }
    c_g[e] = g; c_s[e] = s; c_pair[e] = pair; c_np[e] = np; c_last[e] = frame_no;
    for (np = 0; np < c_np[e]; np++) pair_own[pair + np] = e;
    SMS_mapROMBank(img->bank);
    SMS_VRAMmemcpy((256 + SAT_CACHE + pair * 2) * 32, img->data, c_np[e] * 64);
    upload_budget -= c_np[e];
    return SAT_CACHE + pair * 2;
}

extern s16 pj_dx, pj_dy, pj_depth, pj_lat;
extern u8 pj_ca, pj_sa, pj_cf;
void project(void);

static u8 vis_list[16];
static u8 vis_bot[16], vis_lvl[16];
static s16 vis_cx[16];
static u8 nvis;

static u8 obj_graphic(Obj *o)
{
    u8 g = type_gfx[o->type];
    if (o->type <= OT_BOSS) {
        switch (o->state) {
        case ST_DEAD:   return g + 4;
        case ST_HURT:   return g + 3;
        case ST_ATTACK: return g + 2;
        default:        return g + ((o->anim >> 3) & 1);
        }
    }
    return g;
}

void build_sprites(void)
{
    s16 ca = sintab[(pa + 256) & 1023], sa = sintab[pa & 1023];
    u8 i, j, k;
    Obj *o;
    upload_budget = 40;
    pj_ca = ca < 0 ? (ca < -255 ? 255 : -ca) : (ca > 255 ? 255 : ca);
    pj_sa = sa < 0 ? (sa < -255 ? 255 : -sa) : (sa > 255 ? 255 : sa);
    pj_cf = (ca < 0 ? 1 : 0) | (sa < 0 ? 2 : 0);
    nvis = 0;
    SMS_initSprites();

    /* the paint blaster in hand: drawn first so it is on top */
    SMS_addSprite(VIEW_CX - 3, HORIZON_Y - 7, SAT_CROSS);
    for (i = 0; i < 3; i++)
        for (j = 0; j < 2; j++)
            SMS_addSprite(150 + i * 8 + weapon_frame * 2, 88 + j * 16 + weapon_bob,
                          SAT_WEAPON + weapon_frame * 12 + (i * 2 + j) * 2);

    for (k = 0, o = objs; k < MAXOBJ; k++, o++) {
        s16 dx, dy, depth, lat, sxo;
        u8 hh, want, lvl, h;
        o->vis = 0;
        if (o->type == OT_NONE) continue;
        dx = o->x - px;
        dy = o->y - py;
        if (dx > 0x0C00 || dx < -0x0C00 || dy > 0x0C00 || dy < -0x0C00) continue;
        pj_dx = dx; pj_dy = dy;
        project();
        depth = pj_depth;
        if (depth < 64) continue;
        lat = pj_lat;
        hh = hhtab[(u16)depth >> 3];
        sxo = (s16)mul8(hh, (u16)(lat < 0 ? -lat : lat)) * 2;
        if (sxo > 130) continue;
        if (lat < 0) sxo = -sxo;
        h = type_height[o->type];
        want = (u8)(((u16)h * hh) >> 6 > 255 ? 255 : ((u16)h * hh) >> 6);
        if (want < 5) continue;
        lvl = 0;
        while (lvl < NSIZES - 1 && size_px[lvl + 1] <= want + (want >> 3)) lvl++;
        o->vis = 1;
        o->sx = (u8)(VIEW_CX + sxo < 0 ? 0 : (VIEW_CX + sxo > 255 ? 255 : VIEW_CX + sxo));
        o->halfw = size_px[lvl] >> 1;
        o->depth = depth;
        if (nvis < 16) {
            /* insertion by depth (nearest first) */
            for (j = nvis; j > 0 && objs[vis_list[j - 1]].depth > (u16)depth; j--) {
                vis_list[j] = vis_list[j - 1];
                vis_bot[j] = vis_bot[j - 1];
                vis_lvl[j] = vis_lvl[j - 1];
                vis_cx[j] = vis_cx[j - 1];
            }
            vis_list[j] = k;
            {
                u16 b = HORIZON_Y + hh - (((u16)type_z[o->type] * hh) >> 6);
                vis_bot[j] = b > 200 ? 200 : (u8)b;
            }
            vis_lvl[j] = lvl;
            vis_cx[j] = VIEW_CX + sxo;
            nvis++;
        }
    }

    for (i = 0; i < nvis; i++) {
        u8 lvl = vis_lvl[i];
        u8 g, t, n;
        const SprImg *img;
        const u8 *pc;
        s16 left, top;
        u16 depth;
        o = &objs[vis_list[i]];
        g = obj_graphic(o);
        t = cache_get(g, &lvl);
        if (t == 0xFF) continue;
        img = &spr_imgs[g * NSIZES + lvl];
        left = vis_cx[i] - img->cols * 4;
        top = (s16)vis_bot[i] - img->rows * 16;
        depth = o->depth;
        pc = img->pieces;
        for (n = 0; n < img->npieces; n++, t += 2) {
            u8 c = pc[n];
            s16 x = left + (c & 15) * 8;
            s16 y = top + (c >> 4) * 16;
            s16 ri;
            if (x < VIEW_X0 - 7 || x > VIEW_X0 + 191) continue;
            if (y >= HORIZON_Y + 48 || y < VIEW_TY * 8 - 15) continue;
            ri = (x + 4 - VIEW_X0) >> 2;
            if (ri < 0) ri = 0;
            if (ri > NRAYS - 1) ri = NRAYS - 1;
            if (zbuf[ri] < depth) continue;
            SMS_addSprite((u8)x, (u8)y, t);
        }
    }
    SMS_finalizeSprites();
}
