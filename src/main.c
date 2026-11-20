/* Splat Trooper 3D - a cartoon first-person shooter for the Sega Master System
 *
 * main.c: screens, the frame loop, palette cycling.
 */
#include <SMSlib.h>
#include "defs.h"

SMS_EMBED_SEGA_ROM_HEADER(9999, 0);
SMS_EMBED_SDSC_HEADER_AUTO_DATE(1, 0, "Homebrew", "Splat Trooper 3D", "Cartoon FPS for the Sega Master System");

u8 frame_no;
u8 game_state;          /* 0 title, 1 text screen, 2 playing (for tests) */
u8 fps10;               /* rendered frames in the last 10 s window x1 (debug/tests) */
u16 frames_drawn;
u8 music_busy(void);

static u8 cyc_ph, cyc_last, flash_state;
static u16 cam_x, cam_y, cam_a;

/* ------------------------------------------------------------ palette work */
static void set_cycle(void)
{
    u8 c0 = cycle_cols[cyc_ph];
    u8 c1 = cycle_cols[cyc_ph >= 4 ? cyc_ph - 4 : cyc_ph + 2];
    u8 c2 = cycle_cols[cyc_ph >= 2 ? cyc_ph - 2 : cyc_ph + 4];
    SMS_setBGPaletteColor(13, c0);
    SMS_setBGPaletteColor(14, c1);
    SMS_setBGPaletteColor(15, c2);
    SMS_setSpritePaletteColor(13, c0);
    SMS_setSpritePaletteColor(14, c1);
    SMS_setSpritePaletteColor(15, c2);
}

static void set_skyfloor(u8 mode)
{
    const u8 *p = &level_pals[level * 32];
    u8 i;
    for (i = 0; i < 3; i++) {
        u8 a = p[i], b = p[16 + i];
        if (mode == 1) { a = i == 2 ? 0x1B : 0x17; b = i == 0 ? 0x07 : 0x02; }       /* ouch: pink sky, red floor */
        if (mode == 2) { a = 0x0F; b = 0x0B; }                                          /* pickup: gold */
        SMS_setBGPaletteColor(i, a);
        SMS_setSpritePaletteColor(i, b);
    }
}

static void palette_fx(void)
{
    u8 f;
    if ((u8)(vbl_count >> 3) != cyc_last) {
        cyc_last = vbl_count >> 3;
        if (++cyc_ph == 6) cyc_ph = 0;
        set_cycle();
    }
    f = hurt_flash ? 1 : (pick_flash ? 2 : 0);
    if (f != flash_state) {
        flash_state = f;
        set_skyfloor(f);
    }
}

static void wait_frames(u8 n)
{
    while (n--) SMS_waitForVBlank();
}

static void no_sprites(void)
{
    SMS_initSprites();
    SMS_finalizeSprites();
    SMS_copySpritestoSAT();
}

/* ------------------------------------------------------------ title */
static void title_screen(void)
{
    u8 t = 0, ph = 0, i;
    game_state = 0;
    SMS_displayOff();
    SMS_setSpriteMode(SPRITEMODE_NORMAL);
    SMS_mapROMBank(TITLE_BANK);
    SMS_loadTiles(title_tiles, 0, NTITLE_TILES * 32);
    SMS_loadTileMap(0, 0, title_map, 32 * 24 * 2);
    SMS_loadBGPalette(title_pal);
    SMS_setBackdropColor(0);
    no_sprites();
    SMS_displayOn();
    music_play(mus_title_a, mus_title_b, 1);
    pad_press = 0;
    for (;;) {
        SMS_waitForVBlank();
        t++;
        if ((t & 3) == 0) {
            ph = (ph + 1) & 7;
            for (i = 0; i < 8; i++)
                SMS_setBGPaletteColor(8 + i, title_pal[8 + ((i + ph) & 7)]);
        }
        SMS_setBGPaletteColor(7, (t & 32) ? 0x3F : 0x30);
        if (t > 30 && (pad_press & (PORT_A_KEY_1 | PORT_A_KEY_2))) break;
        pad_press = 0;
    }
    music_stop();
    sfx(SFX_EXIT);
}

/* ------------------------------------------------------------ text screens */
static u16 frame_tile(u8 v)
{
    return (v < 42 ? FRAME_BASE + v : 256 + v - 42) | TILE_PRIORITY;
}

static void text_screen(void)
{
    u8 x, y;
    u16 dot = frame_tile(frame_map[4 * 32]), plain = frame_tile(frame_map[4 * 32 + 1]);
    game_state = 1;
    SMS_displayOff();
    SMS_setSpriteMode(SPRITEMODE_NORMAL);
    SMS_loadTiles(font_tiles, FONT_BASE, 59 * 32);
    SMS_loadTiles(frame_tiles, FRAME_BASE, 42 * 32);
    SMS_loadTiles(frame_tiles + 42 * 32, 256, (NFRAME_TILES - 42) * 32);
    SMS_loadBGPalette(&level_pals[level * 32]);
    SMS_setBackdropColor(BLUE_IDX);
    SMS_setNextTileatXY(0, 0);
    for (y = 0; y < 24; y++)
        for (x = 0; x < 32; x++) {
            if (x >= 2 && x < 30 && y >= 4 && y < 20) SMS_setTile(FONT_BASE | TILE_PRIORITY);
            else SMS_setTile(((x + y) & 1) ? plain : dot);
        }
    no_sprites();
}

static void text_wait(u8 frames)
{
    SMS_displayOn();
    pad_press = 0;
    while (frames--) {
        SMS_waitForVBlank();
        if ((frames & 7) == 0) {
            if (++cyc_ph == 6) cyc_ph = 0;
            set_cycle();
        }
        if (frames < 200 && (pad_press & PORT_A_KEY_1)) break;
    }
}

static void center(u8 y, const char *s)
{
    u8 n = 0;
    while (s[n]) n++;
    print_at((32 - n) >> 1, y, s);
}

/* ------------------------------------------------------------ playing */
static void game_screen(void)
{
    u16 i;
    SMS_displayOff();
    SMS_setSpriteMode(SPRITEMODE_TALL);
    SMS_loadTiles(font_tiles, FONT_BASE, 59 * 32);
    SMS_loadTiles(frame_tiles, FRAME_BASE, 42 * 32);
    SMS_loadTiles(frame_tiles + 42 * 32, 256, (NFRAME_TILES - 42) * 32);
    render_init();
    SMS_setNextTileatXY(0, 0);
    for (i = 0; i < 768; i++) {
        u8 v = frame_map[i];
        u16 t;
        if (v == 0xFF) t = SKY_BASE;
        else t = (v < 42 ? FRAME_BASE + v : 256 + v - 42) | TILE_PRIORITY;
        SMS_setTile(t);
    }
    SMS_loadBGPalette(&level_pals[level * 32]);
    SMS_loadSpritePalette(&level_pals[level * 32 + 16]);
    SMS_setBackdropColor(BLUE_IDX);
    flash_state = 0xFF;
    set_cycle();
    sprites_level_init();
    weapon_frame = 0;
    weapon_bob = 0;
    hud_full();
    cast_rays();
    cam_x = px; cam_y = py; cam_a = pa;
    build_sprites();
    draw_view();
    SMS_copySpritestoSAT();
    game_state = 2;
    SMS_displayOn();
}

static void do_pause(void)
{
    print_at(13, 1, "PAUSED");
    music_stop();
    SMS_resetPauseRequest();
    while (!SMS_queryPauseRequested()) SMS_waitForVBlank();
    SMS_resetPauseRequest();
    print_at(13, 1, "      ");
    music_play(mus_game_a, mus_game_b, 1);
}

static u8 play_level(void)
{
    u8 last, n, fc = 0, t0;
    level_load(level);
    game_screen();
    music_play(mus_game_a, mus_game_b, 1);
    last = vbl_count;
    t0 = vbl_count;
    for (;;) {
        n = vbl_count - last;
        last += n;
        if (n > 6) n = 6;
        if (n) game_tick(n);
        if (game_result) break;
        if (px != cam_x || py != cam_y || pa != cam_a) {
            cam_x = px; cam_y = py; cam_a = pa;
            cast_rays();
        }
        build_sprites();
        draw_view();
        SMS_waitForVBlank();
        SMS_copySpritestoSAT();
        palette_fx();
        hud_draw();
        frame_no++;
        frames_drawn++;
        if (++fc == 60) { fps10 = (u8)(3600 / (u8)(vbl_count - t0)); fc = 0; t0 = vbl_count; }
        if (SMS_queryPauseRequested()) { do_pause(); last = vbl_count; }
    }
    music_stop();
    hud_draw();
    /* a short beat so the final hit / the portal is seen */
    wait_frames(40);
    return game_result;
}

static void play_game(void)
{
    u8 r;
    for (;;) {
        text_screen();
        center(8, "LEVEL");
        print_num(19, 8, level + 1, 1);
        center(11, level_names[level]);
        center(15, "SPLAT 'EM ALL!");
        center(17, "FIND THE PORTAL");
        text_wait(150);
        r = play_level();
        if (r == 1) {
            text_screen();
            center(8, "LEVEL CLEAR!");
            center(12, "SCORE");
            print_num(13, 14, score, 5);
            print_at(19, 14, "PTS");
            score += 0;
            music_play(mus_clear_a, mus_clear_b, 0);
            text_wait(150);
            level++;
            if (level == NLEVELS) {
                level = 0;
                text_screen();
                center(6, "HOORAY!");
                center(9, "DR. GLOOP IS BEATEN");
                center(11, "TOONTOWN IS SAFE!");
                center(15, "FINAL SCORE");
                print_num(13, 17, score, 5);
                music_play(mus_clear_a, mus_clear_b, 1);
                text_wait(255);
                text_wait(255);
                music_stop();
                return;
            }
        } else {
            lives--;
            text_screen();
            if (lives == 0) {
                center(9, "GAME OVER");
                center(13, "SCORE");
                print_num(13, 15, score, 5);
                music_play(mus_over_a, mus_over_b, 0);
                text_wait(220);
                return;
            }
            center(9, "OUCH! SPLATTED!");
            center(12, "TROOPERS LEFT");
            print_num(15, 14, lives, 1);
            music_play(mus_over_a, mus_over_b, 0);
            text_wait(150);
            health = 100;
            if (ammo < 30) ammo = 30;
        }
    }
}

void main(void)
{
    snd_init();
    SMS_setFrameInterruptHandler(snd_isr);
    for (;;) {
        title_screen();
        game_new();
        play_game();
    }
}
