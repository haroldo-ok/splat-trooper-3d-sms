/* game.c - player, monsters, projectiles, pickups */
#include <SMSlib.h>
#include "defs.h"

u16 px, py, pa;
u8 map[1024];
Obj objs[MAXOBJ];
u8 level, lives, health, ammo;
u16 score;
u8 game_result;
u8 hurt_flash, pick_flash, face_ouch;
u8 hud_dirty;
u8 kills, nmonsters, boss_alive, msg_timer;
static u8 fire_cool;
static u8 tickno;
static u8 tn;        /* ticks (1/60 s) covered by this game update */

static u8 dec_sat(u8 v, u8 n) { return v > n ? v - n : 0; }

#define PRAD 0x50

/* enemy stats per type: hp, speed, attack range (1/16 tile), cooldown, damage */
static const u8 e_hp[]    = { 3, 4, 8, 20 };
static const u8 e_speed[] = { 9, 12, 7, 6 };
static const u8 e_range[] = { 14, 112, 128, 144 };
static const u8 e_cool[]  = { 50, 120, 80, 45 };
static const u8 e_dmg[]   = { 7, 8, 10, 12 };
static const u8 e_score[] = { 10, 20, 40, 250 };

static u8 cell(u16 x, u16 y)
{
    return map[((y >> 8) << 5) | (x >> 8)];
}

static Obj *new_obj(void)
{
    u8 i;
    for (i = 0; i < MAXOBJ; i++)
        if (objs[i].type == OT_NONE) {
            objs[i].state = 0; objs[i].timer = 0; objs[i].anim = i << 2;
            objs[i].cool = 30 + i * 3; objs[i].vx = objs[i].vy = 0;
            objs[i].vis = 0;
            return &objs[i];
        }
    return 0;
}

void level_load(u8 lvl)
{
    const u8 *m, *ob;
    u8 i;
    SMS_mapROMBank(MISC_BANK);
    m = lvl == 0 ? lvl0_map : lvl == 1 ? lvl1_map : lvl2_map;
    ob = lvl == 0 ? lvl0_objs : lvl == 1 ? lvl1_objs : lvl2_objs;
    for (i = 0; ; i++) {
        map[i] = m[i];
        map[i + 256] = m[i + 256];
        map[i + 512] = m[i + 512];
        map[i + 768] = m[i + 768];
        if (i == 255) break;
    }
    for (i = 0; i < MAXOBJ; i++) objs[i].type = OT_NONE;
    nmonsters = 0;
    boss_alive = 0;
    msg_timer = 0;
    kills = 0;
    while (*ob != 0xFF) {
        Obj *o = new_obj();
        if (!o) break;
        o->type = ob[0];
        o->x = ((u16)ob[1] << 8) | 0x80;
        o->y = ((u16)ob[2] << 8) | 0x80;
        if (o->type <= OT_BOSS) {
            o->hp = e_hp[o->type];
            nmonsters++;
            if (o->type == OT_BOSS) boss_alive = 1;
        }
        ob += 3;
    }
    px = ((u16)level_starts[lvl * 2] << 8) | 0x80;
    py = ((u16)level_starts[lvl * 2 + 1] << 8) | 0x80;
    pa = 0;
    game_result = 0;
    fire_cool = 0;
    hurt_flash = pick_flash = face_ouch = 0;
    hud_dirty = 0x0F;
}

void game_new(void)
{
    level = 0;
    lives = 3;
    score = 0;
    health = 100;
    ammo = 50;
}

/* ------------------------------------------------------------ helpers */
static u16 approx_dist(s16 dx, s16 dy)
{
    u16 ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
    return ax > ay ? ax + (ay >> 1) - (ay >> 3) : ay + (ax >> 1) - (ax >> 3);
}

/* line of sight between two points, sampling every 1/4 tile */
static u8 los(u16 x0, u16 y0, u16 x1, u16 y1)
{
    s16 dx = (s16)(x1 - x0), dy = (s16)(y1 - y0);
    u16 d = approx_dist(dx, dy);
    u8 n = (u8)(d >> 6) + 1, i;
    s16 sx = dx / n, sy = dy / n;
    for (i = 1; i < n; i++) {
        x0 += sx; y0 += sy;
        if (cell(x0, y0)) return 0;
    }
    return 1;
}

static u8 blocked(u16 x, u16 y, u16 r)
{
    return cell(x - r, y - r) || cell(x + r, y - r) || cell(x - r, y + r) || cell(x + r, y + r);
}

static u8 try_move(u16 *x, u16 *y, s16 mx, s16 my, u16 r)
{
    u8 bumped = 0;
    if (mx) {
        if (!blocked(*x + mx, *y, r)) *x += mx; else bumped = 1;
    }
    if (my) {
        if (!blocked(*x, *y + my, r)) *y += my; else bumped |= 2;
    }
    return bumped;
}

void hurt_player(u8 dmg)
{
    if (health == 0) return;
    health = health > dmg ? health - dmg : 0;
    hurt_flash = 10;
    face_ouch = 20;
    hud_dirty |= 1;
    sfx(SFX_HURT);
    if (health == 0) game_result = 2;
}

static void spawn_splat(u16 x, u16 y)
{
    Obj *o = new_obj();
    if (!o) return;
    o->type = OT_SPLAT;
    o->x = x; o->y = y;
    o->timer = 14;
}

/* ------------------------------------------------------------ firing */
void game_fire(void)
{
    u8 i, best = 0xFF;
    u16 bd = center_dist;
    Obj *o;
    if (ammo == 0 || fire_cool) return;
    ammo--;
    hud_dirty |= 2;
    fire_cool = 14;
    weapon_frame = 1;
    sfx(SFX_SHOOT);
    for (i = 0, o = objs; i < MAXOBJ; i++, o++) {
        if (o->type > OT_BOSS || o->state == ST_DEAD || !o->vis) continue;
        if (o->depth < bd) {
            u8 hw = o->halfw > 6 ? o->halfw - 2 : 4;
            if (o->sx + hw >= VIEW_CX && o->sx <= VIEW_CX + hw) {
                bd = o->depth;
                best = i;
            }
        }
    }
    if (best != 0xFF) {
        o = &objs[best];
        o->state = ST_HURT;
        o->timer = 10;
        spawn_splat(o->x - (s16)(sintab[(pa + 256) & 1023] >> 2), o->y - (s16)(sintab[pa] >> 2));
        if (o->hp) o->hp--;
        if (o->hp == 0) {
            o->state = ST_DEAD;
            o->timer = 0;
            score += e_score[o->type];
            kills++;
            if (o->type == OT_BOSS) { boss_alive = 0; score += 500; }
            hud_dirty |= 4;
            sfx(SFX_DIE);
        } else {
            sfx(SFX_HIT);
        }
    } else {
        /* paint splat on the wall straight ahead */
        u16 d = center_dist > 0x40 ? center_dist - 0x30 : 0x10;
        s16 ca = sintab[(pa + 256) & 1023], sa = sintab[pa];
        u8 ac = ca < 0 ? (u8)(-ca > 255 ? 255 : -ca) : (u8)(ca > 255 ? 255 : ca);
        u8 as = sa < 0 ? (u8)(-sa > 255 ? 255 : -sa) : (u8)(sa > 255 ? 255 : sa);
        s16 ox = (s16)mul8(ac, d), oy = (s16)mul8(as, d);
        spawn_splat(px + (ca < 0 ? -ox : ox), py + (sa < 0 ? -oy : oy));
    }
}

/* ------------------------------------------------------------ monsters */
static void shoot_at_player(Obj *e)
{
    Obj *b = new_obj();
    s16 dx, dy;
    u16 d;
    if (!b) return;
    b->type = e->type == OT_IMP ? OT_FIREBALL : OT_PLASMA;
    b->x = e->x; b->y = e->y;
    dx = (s16)(px - e->x); dy = (s16)(py - e->y);
    d = approx_dist(dx, dy) >> 4;
    if (d == 0) d = 1;
    /* ~ 1/16 tile per tick (fireball) / 5/64 (plasma) */
    b->vx = (dx / (s16)d) * (b->type == OT_FIREBALL ? 1 : 1);
    b->vy = (dy / (s16)d);
    if (b->type == OT_PLASMA) { b->vx += b->vx >> 2; b->vy += b->vy >> 2; }
    b->timer = 200;
    b->hp = e_dmg[e->type];
    sfx(SFX_ESHOT);
}

static void monster_tick(Obj *e, u8 idx)
{
    s16 dx = (s16)(px - e->x), dy = (s16)(py - e->y);
    u16 d = approx_dist(dx, dy);
    u8 t = e->type;
    e->anim += tn;
    e->cool = dec_sat(e->cool, tn);
    switch (e->state) {
    case ST_DEAD:
        return;
    case ST_HURT:
    case ST_ATTACK:
        e->timer = dec_sat(e->timer, tn);
        if (e->timer == 0) e->state = ST_CHASE;
        return;
    case ST_IDLE:
        if (((tickno + idx) & 15) < tn && d < 0x0A00 && los(e->x, e->y, px, py))
            e->state = ST_CHASE;
        return;
    }
    /* chase */
    if (t == OT_BLOB) {
        if (d < (u16)e_range[t] << 4) {
            if (!e->cool) {
                e->state = ST_ATTACK; e->timer = 16; e->cool = e_cool[t];
                hurt_player(e_dmg[t]);
            }
            return;
        }
    } else if (!e->cool && d < (u16)e_range[t] << 4 && ((tickno + idx) & 3) < tn &&
               los(e->x, e->y, px, py)) {
        e->state = ST_ATTACK; e->timer = 18; e->cool = e_cool[t] + (tickno & 15);
        shoot_at_player(e);
        return;
    }
    /* walk toward the player, keeping a little distance */
    if (d > (t == OT_BLOB ? 0xC0 : 0x1C0)) {
        u8 sp = e_speed[t] * tn;
        s16 mx = 0, my = 0;
        u16 ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
        if (ax > 0x20) mx = dx < 0 ? -sp : sp;
        if (ay > 0x20) my = dy < 0 ? -sp : sp;
        if (mx && my && (ax > ay * 2 || ay > ax * 2)) {
            if (ax > ay) my >>= 1; else mx >>= 1;
        }
        if (try_move(&e->x, &e->y, mx, my, 0x40) == 3) {
            /* stuck on a corner: slide sideways a bit */
            try_move(&e->x, &e->y, (e->anim & 32) ? sp : -sp, (e->anim & 32) ? -sp : sp, 0x40);
        }
    }
}

static void projectile_tick(Obj *b)
{
    s16 dx, dy;
    b->x += b->vx * tn;
    b->y += b->vy * tn;
    b->timer = dec_sat(b->timer, tn);
    if (cell(b->x, b->y) || b->timer == 0) {
        b->type = OT_NONE;
        return;
    }
    dx = (s16)(px - b->x); dy = (s16)(py - b->y);
    if (dx > -0x50 && dx < 0x50 && dy > -0x50 && dy < 0x50) {
        hurt_player(b->hp);
        b->type = OT_NONE;
    }
}

static void pickup_tick(Obj *o)
{
    s16 dx = (s16)(px - o->x), dy = (s16)(py - o->y);
    if (dx > -0x70 && dx < 0x70 && dy > -0x70 && dy < 0x70) {
        if (o->type == OT_HEART) {
            if (health >= 100) return;
            health = health > 75 ? 100 : health + 25;
            hud_dirty |= 1;
            sfx(SFX_PICK);
        } else if (o->type == OT_AMMO) {
            if (ammo >= 99) return;
            ammo = ammo > 79 ? 99 : ammo + 20;
            hud_dirty |= 2;
            sfx(SFX_PICK);
        } else {
            score += 100;
            hud_dirty |= 4;
            sfx(SFX_STAR);
        }
        pick_flash = 6;
        o->type = OT_NONE;
    }
}

/* ------------------------------------------------------------ the tick */
void game_tick(u8 n)
{
    u8 keys = pad_held;
    u8 i;
    Obj *o;
    s16 ca, sa;
    tn = n;
    tickno += n;
    if (fire_cool) {
        fire_cool = dec_sat(fire_cool, n);
        if (fire_cool < 8) weapon_frame = 0;
    }
    if (msg_timer) { msg_timer = dec_sat(msg_timer, n); if (!msg_timer) hud_dirty |= 32; }
    hurt_flash = dec_sat(hurt_flash, n);
    pick_flash = dec_sat(pick_flash, n);
    if (face_ouch) { face_ouch = dec_sat(face_ouch, n); if (!face_ouch) hud_dirty |= 8; }

    if (keys & PORT_A_KEY_2) {
        /* strafe along the right-hand vector */
        s16 rx = (sintab[(pa + 512) & 1023] * 3) >> 6;
        s16 ry = (sintab[(pa + 256) & 1023] * 3) >> 6;
        rx *= n; ry *= n;
        if (keys & PORT_A_KEY_LEFT) try_move(&px, &py, -rx, -ry, PRAD);
        if (keys & PORT_A_KEY_RIGHT) try_move(&px, &py, rx, ry, PRAD);
    } else {
        if (keys & PORT_A_KEY_LEFT) pa = (pa - 7 * n) & 1023;
        if (keys & PORT_A_KEY_RIGHT) pa = (pa + 7 * n) & 1023;
    }
    ca = sintab[(pa + 256) & 1023];
    sa = sintab[pa];
    if (keys & (PORT_A_KEY_UP | PORT_A_KEY_DOWN)) {
        s16 mx = ((ca * 3) >> 6) * n, my = ((sa * 3) >> 6) * n;   /* 3/64 tile per tick */
        u8 b;
        if (keys & PORT_A_KEY_DOWN) { mx = -mx; my = -my; }
        b = try_move(&px, &py, mx, my, PRAD);
        if (b) {
            /* bumping the portal clears the level */
            u16 tx = px + mx + (mx > 0 ? PRAD + 4 : mx < 0 ? -(PRAD + 4) : 0);
            u16 ty = py + my + (my > 0 ? PRAD + 4 : my < 0 ? -(PRAD + 4) : 0);
            if (cell(tx, py) == 6 || cell(px, ty) == 6 || cell(tx, ty) == 6) {
                if (boss_alive) {
                    if (!msg_timer) { msg_timer = 120; hud_dirty |= 16; }
                } else {
                    game_result = 1;
                    sfx(SFX_EXIT);
                }
            }
        }
        weapon_bob = (tickno >> 2) & 3;
        if (weapon_bob == 3) weapon_bob = 1;
    }
    if ((keys & PORT_A_KEY_1) && !fire_cool) game_fire();

    for (i = 0, o = objs; i < MAXOBJ; i++, o++) {
        u8 t = o->type;
        if (t == OT_NONE) continue;
        if (t <= OT_BOSS) monster_tick(o, i);
        else if (t <= OT_STAR) pickup_tick(o);
        else if (t <= OT_PLASMA) projectile_tick(o);
        else { o->timer = dec_sat(o->timer, n); if (!o->timer) o->type = OT_NONE; }
    }
}
