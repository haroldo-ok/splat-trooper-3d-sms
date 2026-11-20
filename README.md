# Splat Trooper 3D

A cartoon first-person shooter for the **Sega Master System**. You play a paint-blaster trooper clearing
Dr. Gloop's goo monsters out of Toontown: three levels, three kinds of monster plus a boss robot, pickups, a
HUD with a face that reacts, music and sound effects.

`splat_trooper_3d.sms`: a 256 KB ROM with the standard Sega mapper. It needs no cartridge RAM and no
sprite zoom.

## Controls

| pad | action |
|---|---|
| Up / Down | walk forward / back |
| Left / Right | turn |
| Button 2 + Left / Right | strafe |
| Button 1 | fire (hold it down for auto-fire) |
| Pause (console) | pause |

To finish a level, walk into the swirling rainbow **portal**. On level 3 the portal stays shut until
Dr. Gloop's Mega-Bot is splatted. Hearts give back health, paint cans give ammo, and stars give points. You
start with 3 troopers.

## Where it comes from

The engine design follows haroldo-ok's **wolf-3d-sms**:
- a 192 x 96 view of 48 rays, each 4 px wide, with two rays per tile column
- a tile per view cell, built in RAM and copied to VRAM with paced writes
- shared flat tiles for cells that are all sky
- an "on-screen signature" per column, so unchanged columns are skipped
- generated, unrolled column scalers

The code here is new, and so are all the assets: textures, characters, title, font, HUD, maps and music are
drawn or written by `tools/gen.py`. Nothing from the reference repo's art or data was used.

### What is new compared with the reference

**Vertically mirrored walls, using the background V-flip.** Every wall texture is symmetric from top to
bottom, so only its top half (32 x 16) is stored. The renderer builds only the **top 6 of the 12 tile rows**
of the view. In the name table, the bottom 6 rows point at the *same* tiles with `TILE_FLIPPED_Y` set.

The bottom rows also have `TILE_USE_SPRITE_PALETTE` set. The sprite palette is the BG palette with entries
0–2 changed from sky colours to floor colours, so the mirrored sky becomes a floor gradient. This halves the
column-building work and halves the VRAM upload.

Measured in the test emulator, compared with a benchmark build that also pays for the bottom half
(`SIMFULL = 1` in `src/ray.s`):

| scenario | full-height cost | mirrored | gain |
|---|---|---|---|
| turning | 7.6 fps | 9.8 fps | +29% |
| walking | 10.6 fps | 15.4 fps | +45% |
| mixed play (walk, turn, fight) | 4.7 fps | 7.5 fps | +61% |
| standing still | 31 fps | 31 fps | – |

The wall work itself is halved. The total gain is smaller than 2x because ray casting, sprites and game
logic don't shrink.

**Pre-scaled hardware sprites.** Each monster, pickup and projectile frame is drawn offline at 8 sizes from
8 to 48 px. At run time the nearest size is picked and shown with 8x16 hardware sprites, so no sprite pixels
are drawn into the background. A small VRAM cache uploads an image only when it first appears.

Walls hide sprites per 8 px column using the ray depth buffer. The frame and HUD tiles have the priority bit
set, so sprites never spill outside the view.

**Palette rotation.** Palette entries 13–15 cycle through a 6-colour candy rainbow, which animates:
- the candy-stripe walls, the portal swirl and the glowing goo on the hedges
- the monsters' glowing bits and the paint in the blaster's tank
- the plasma shots and the polka dots of the frame

The title screen also cycles 8 entries, which scrolls its diagonal rainbow, and blinks "PRESS BUTTON 1".
Hits and pickups briefly flash the sky and floor entries.

**Cartoon look.** All art has black outlines and flat, bright colours. Each level has its own sky and floor
palette: sunny day, sunset and a night lab.

### Engine notes

- `src/ray.s`: ray casting and view composition.
  - **Casting:** the DDA inner loop runs from RAM, with its step constants patched for each ray. An odd ray
    whose two neighbours hit the same face of the same cell is interpolated instead of cast. A per-angle
    record table in a ROM bank holds 1/cos, 1/sin and the signs.
  - **Composition:** a per-column signature check, the name-table switch between sky tiles and column
    tiles, and the mirrored bottom half.
- `src/asm.s`: the column builders and the copy to VRAM.
  - `draw_left`/`draw_right` jump into unrolled scalers, generated as `gen/scalers.s`.
  - Each wall texture bank holds texel *rows as pages*, the sky colours, and the row tables for 138 wall
    heights.
  - `out_column` turns pixel pairs into bitplanes with a 1 KB lookup table in RAM, at exactly 27 cycles per
    VRAM write.
  - `mul8` is a quarter-square multiply.
- Game time doesn't depend on frame rate. The frame interrupt counts VBlanks and latches the pad, and the
  game advances by the ticks that have passed (capped at 6).
- `src/sound.c`: a small PSG driver in the frame interrupt. Two music channels; sound effects on tone
  channel 2 and the noise channel.

## Building

You need SDCC 4.2 or later, devkitSMS (SMSlib), and python3 with Pillow:

    make            # runs tools/gen.py, then builds splat_trooper_3d.sms
    tests/run_all.sh

`tools/gen.py` refuses to build a level unless a flood fill from the start reaches every object and the exit portal.

`tools/checkmap.py` runs after each link. It checks that:
- the code fits in the fixed 32 KB
- RAM stays below the page-aligned tables at 0xD000
- the texture banks start at 0x8000

## Tests

The playtests in `tests/` run on `smstest`, the headless SMS emulator that comes with the devkitSMS skill:

| test | what it checks |
|---|---|
| boot | boot, title, entering the game |
| walkout | controller-only: leaving the start room of level 1 |
| fight | killing a blob |
| flow | monsters hurt you; portals to levels 2 and 3; death |
| boss | the portal is locked until the boss dies; victory; back to the title |
| over | pause, game over, back to the title |
| soak | 1800 frames of controller-only play |
| sound | PSG output |
| text | the level intro screen |

The soak test also checks that no VRAM write is spaced too tightly while the display is on, and checks the
stack depth.

**Not verified:** real hardware, and other emulators. Only `smstest` has run this ROM.
