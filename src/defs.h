/* Splat Trooper 3D - shared definitions */
#ifndef DEFS_H
#define DEFS_H

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;

typedef struct {
    u8 bank;
    const u8 *data;
    u8 npieces, cols, rows;
    const u8 *pieces;
} SprImg;

/* ---- fixed RAM areas (page aligned, used by the asm) ---- */
#define COLBUF   ((u8 *)0xD700)    /* 48 bytes: one tile column, packed L|R pixel pairs */
#define LUT      ((u8 *)0xD800)    /* 4 x 256: pixel pair -> bitplane byte */

/* ---- screen layout ---- */
#define VIEW_TX   4
#define VIEW_TY   3
#define VIEW_TW   24
#define VIEW_TH   12               /* only the top 6 rows are rendered */
#define NRAYS     48
#define HORIZON_Y 72               /* screen y of the horizon */
#define VIEW_X0   32
#define VIEW_CX   128
#define BLUE_IDX  12

/* ---- BG tile allocation ---- */
#define FONT_BASE  0               /* ASCII 32..90 */
#define VIEW_BASE  64              /* 24 columns x 6 rows (tc*6 + r) */
#define SKY_BASE   208             /* 6 sky tiles (top half rows) */
#define FRAME_BASE 214             /* 42 tiles here, then 256.. */
/* sprite area: SAT tile n == BG tile 256+n */
#define SAT_FACE    40
#define SAT_WEAPON  56             /* 2 frames x 12 tiles */
#define SAT_CROSS   80
#define SAT_CACHE   82             /* 110 tiles for the pre-scaled image cache */
#define CACHE_PAIRS 55

/* ---- objects ---- */
enum { OT_BLOB, OT_IMP, OT_ROBOT, OT_BOSS, OT_HEART, OT_AMMO, OT_STAR,
       OT_FIREBALL, OT_PLASMA, OT_SPLAT, OT_NONE = 0xFF };
enum { ST_IDLE, ST_CHASE, ST_ATTACK, ST_HURT, ST_DEAD };

typedef struct {
    u16 x, y;          /* 8.8 tiles */
    u8 type;
    u8 state;
    u8 hp;
    u8 timer;
    u8 anim;
    u8 cool;
    s16 vx, vy;        /* projectiles */
    /* filled by the sprite projection each frame */
    u8 vis;            /* 1: on screen last frame */
    u8 sx;             /* screen centre x */
    u8 halfw;
    u16 depth;
} Obj;

#define MAXOBJ 40

/* ---- globals ---- */
extern u16 px, py, pa;
extern u8 map[1024];
extern Obj objs[MAXOBJ];
extern u16 zbuf[NRAYS];
extern u8 frame_no;
extern volatile u8 vbl_count;
extern volatile u8 pad_held, pad_press;

/* render.c */
void render_init(void);
void render_invalidate(void);
void cast_rays(void);
void draw_view(void);
extern u16 center_dist;

/* asm */
extern u8 mq_a; extern u16 mq_b, mq_r;
void mulq(void);
u16 mul8(u8 a, u16 b);
extern u8 rd_bank, rd_l, rd_y0;
extern u16 rd_rows, rd_vaddr;
void draw_left(void);
void draw_right(void);
void out_column(void);

/* sprites.c */
void sprites_level_init(void);
void build_sprites(void);
extern u8 weapon_frame;
extern s8 weapon_bob;

/* game.c */
void game_new(void);
void level_load(u8 lvl);
void game_tick(u8 n);
void game_fire(void);
extern u8 level, lives, health, ammo;
extern u16 score;
extern u8 game_result;      /* 0 playing, 1 cleared, 2 died */
extern u8 hurt_flash, pick_flash, face_ouch;
extern u8 hud_dirty;
extern u8 msg_timer, boss_alive;

/* hud.c */
void print_at(u8 x, u8 y, const char *s);
void print_num(u8 x, u8 y, u16 v, u8 digits);
void hud_draw(void);
void hud_full(void);

/* sound.c */
void snd_init(void);
void snd_isr(void);
void music_play(const u8 *a, const u8 *b, u8 loop);
void music_stop(void);
void sfx(u8 id);
enum { SFX_SHOOT = 1, SFX_HIT, SFX_PICK, SFX_HURT, SFX_DIE, SFX_ESHOT, SFX_EXIT, SFX_STAR };

/* generated tables / assets */
#include "../gen/assets.h"
extern const short sintab[1024];

extern const signed char rayoff[NRAYS];
extern const unsigned char cosoff[NRAYS];
extern const unsigned char hhtab[1536];

extern const unsigned char plane_lut[1024];
extern const unsigned char sky_tiles[192];
extern const SprImg spr_imgs[];
extern const unsigned char font_tiles[];
extern const unsigned char frame_tiles[];
extern const unsigned char frame_map[768];
extern const unsigned char level_pals[];
extern const unsigned char cycle_cols[6];
extern const unsigned char title_pal[16];
extern const char * const level_names[];
extern const unsigned char level_starts[];
extern const unsigned char mus_title_a[], mus_title_b[], mus_game_a[], mus_game_b[];
extern const unsigned char mus_clear_a[], mus_clear_b[], mus_over_a[], mus_over_b[];
/* banked */
extern const unsigned char lvl0_map[], lvl1_map[], lvl2_map[];
extern const unsigned char lvl0_objs[], lvl1_objs[], lvl2_objs[];
extern const unsigned char face_tiles[], weapon_tiles[];
extern const unsigned char title_tiles[];
extern const unsigned short title_map[];

#endif
