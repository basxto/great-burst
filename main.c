// original gameboy
#include <gb/gb.h>
// gameboy color
#include <gb/cgb.h>
#include <stdio.h>

#include "great_burst.h"
#include "island_joy_16.h"
#include "menu.h"
#include "splashscreen.h"

const UWORD splash_bg_palette[] = {IJ16_WHITE, IJ16_PINK, IJ16_GREY,
                                   IJ16_BLACK};
const UWORD splash_fg_palette[] = {0, IJ16_PINK, IJ16_GREY,  IJ16_BLACK,
                                   0, IJ16_PINK, IJ16_WHITE, IJ16_BLACK};

const UWORD bg_palette[] = {
    // metal
    IJ16_YELLOW, IJ16_BEIGE, IJ16_GREY, IJ16_BLACK,
    // blue block
    IJ16_TURQUOISE, IJ16_BLUE, IJ16_DARK_GREEN, IJ16_BLACK,
    // green block
    IJ16_LIGHT_GREEN, IJ16_GREEN, IJ16_DARK_GREEN, IJ16_BLACK,
    // orange block
    IJ16_YELLOW, IJ16_ORANGE, IJ16_DARK_PINK, IJ16_BLACK,
    // pink block
    IJ16_LIGHT_PINK, IJ16_PINK, IJ16_DARK_PINK, IJ16_BLACK,
    // ball block
    IJ16_YELLOW, IJ16_ORANGE, IJ16_GREY, IJ16_BLACK,
    // tnt
    IJ16_ORANGE, IJ16_RED, IJ16_RED, IJ16_BLACK,
    // rod
    IJ16_YELLOW, IJ16_BLUE, IJ16_GREY, IJ16_BLACK};

const UWORD fg_palette[] = {
    // metal
    0, IJ16_BEIGE, IJ16_GREY, IJ16_BLACK,
    // metal reflection
    0, IJ16_BEIGE, IJ16_GREY, IJ16_YELLOW,
    // green paddle
    0, IJ16_LIGHT_GREEN, IJ16_DARK_GREEN, IJ16_BLACK,
    // fire
    0, IJ16_ORANGE, IJ16_DARK_PINK, IJ16_BLACK};

void main() {
    NR52_REG = 0x80; // enable sound
    NR50_REG = 0x77; // full volume
    NR51_REG = 0xFF; // all channels

    cgb_compatibility();
    set_sprite_palette(0, 4, fg_palette);
    set_bkg_palette(0, 8, bg_palette);

    set_sprite_palette(0, 3, splash_fg_palette);
    set_bkg_palette(0, 1, splash_bg_palette);
    splash();
    set_sprite_palette(0, 4, fg_palette);
    set_bkg_palette(0, 8, bg_palette);
    menu(0);
}
