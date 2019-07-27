#include "splashscreen.h"

#include "background.c"

#include "pix/oga_splash_data.c"
#include "pix/oga_splash_map.c"
// only O and G
#include "pix/oga_splash_map_og.c"
// movable parts of the logo
#include "pix/oga_splash_movable_data.c"
#include "pix/oga_splash_movable_map.c"

// make a bling sound with three notes
void bling() {
    NR10_REG = 0x00; // no sweep
    NR11_REG = 0x50; // 50% duty

    NR12_REG = 0xF0; // constant volume envelope
    NR14_REG = 0xC0 | ((notes[4][note_d] >> 8) & 0x07); // msb
    NR13_REG = notes[4][note_d] & 0xFF;                 // lsb
    wait_vbl_done();
    wait_vbl_done();
    wait_vbl_done();
    wait_vbl_done();
    NR12_REG = 0xF0;
    NR14_REG = 0xC0 | ((notes[4][note_f] >> 8) & 0x07);
    NR13_REG = notes[4][note_f] & 0xFF;
    wait_vbl_done();
    wait_vbl_done();
    wait_vbl_done();
    wait_vbl_done();
    wait_vbl_done();
    wait_vbl_done();
    wait_vbl_done();
    wait_vbl_done();
    NR12_REG = 0xF1; // falling volume envelope
    NR14_REG = 0xC0 | ((notes[5][note_cis] >> 8) & 0x07);
    NR13_REG = notes[5][note_cis] & 0xFF;
}

// moves head in 'background' array
// starts on 5th (of 18) row, 2nd (of 20) column
// works from right to left and must operate this way
// 0x00 is a blank tile
void move_head(UINT8 offset) {
    UINT8 j, i;
    if (offset > 0) {
        // move sticking out pieces from colums 7 and 8
        background[5 * 20 + 6 + offset] = 0x00;
        background[6 * 20 + 6 + offset] = 0x00;
        background[7 * 20 + 6 + offset] = background[7 * 20 + 6];
        background[7 * 20 + 6] = 0x00;
        background[8 * 20 + 6 + offset] = background[8 * 20 + 6];
        background[8 * 20 + 6] = 0x00;
        background[6 * 20 + 7 + offset] = 0x00;
        background[7 * 20 + 7 + offset] = background[7 * 20 + 7];
        background[7 * 20 + 7] = 0x00;
        // move remaining columns 2-6
        for (i = 5; i > 0; --i) {     // change column
            for (j = 0; j < 6; ++j) { // go through column
                background[(j + 4) * 20 + i + offset] =
                    background[(j + 4) * 20 + i];
                background[(j + 4) * 20 + i] = 0x00;
            }
        }
    }
}

void splash() {
    UINT8 i, j, index;
    // set second color palette
    // dark grey and white get switched
    OBP1_REG = 0xC6; // 11000110
    set_bkg_data(0, 150, oga_splash_data);
    memcpy(background, oga_splash_map_og, 360);

    // move O over G
    move_head(5);
    set_bkg_tiles(0, 0, 20, 18, background);
    SHOW_BKG;

    set_sprite_data(0, 59, oga_splash_movable_data);

    // load rest of O as sprites
    set_sprite_tile(0, 0);
    move_sprite(0, 1 << 3, 0);
    set_sprite_tile(1, 1);
    move_sprite(1, 1 << 3, 1 << 3);
    set_sprite_tile(2, 2);
    move_sprite(2, 2 << 3, 2 << 3);
    set_sprite_tile(3, 3);
    move_sprite(3, 2 << 3, 3 << 3);
    set_sprite_tile(4, 4);
    move_sprite(4, 1 << 3, 4 << 3);
    set_sprite_tile(5, 5);
    move_sprite(5, 1 << 3, 5 << 3);
    set_sprite_tile(6, 9);
    move_sprite(6, 0, 0);
    set_sprite_tile(7, 10);
    move_sprite(7, 0, 5 << 3);
    // move over G and change palette
    for (i = 0; i < 8; ++i) {
        scroll_sprite(i, 12 << 3, 6 << 3);
        set_sprite_prop(i, S_PALETTE);
    }

    // load A as sprite; 6x6 pixel
    // 1-2
    for (i = 2; i <= 3; ++i) {
        set_sprite_tile((i + 6),
                        oga_splash_movable_map[splashscreen_a_xoffset + i]);
        move_sprite((i + 6), i << 3, 0);
    }
    // 3-6
    for (i = 1; i <= 4; ++i) {
        set_sprite_tile((i + 9),
                        oga_splash_movable_map[splashscreen_line_width +
                                               splashscreen_a_xoffset + i]);
        move_sprite((i + 9), i << 3, 1 << 3);
    }
    // 7-10
    for (i = 1; i <= 4; ++i) {
        set_sprite_tile((i + 13),
                        oga_splash_movable_map[splashscreen_line_width * 2 +
                                               splashscreen_a_xoffset + i]);
        move_sprite((i + 13), i << 3, 2 << 3);
    }
    // 11-16
    for (i = 0; i <= 5; ++i) {
        set_sprite_tile((i + 18),
                        oga_splash_movable_map[splashscreen_line_width * 3 +
                                               splashscreen_a_xoffset + i]);
        move_sprite((i + 18), i << 3, 3 << 3);
    }

    // 17-22
    for (i = 0; i <= 5; ++i) {
        set_sprite_tile((i + 24),
                        oga_splash_movable_map[splashscreen_line_width * 4 +
                                               splashscreen_a_xoffset + i]);
        move_sprite((i + 24), i << 3, 4 << 3);
    }

    // 23-24 at index 13 and 14
    set_sprite_tile(30, oga_splash_movable_map[splashscreen_line_width * 5 +
                                               splashscreen_a_xoffset]);
    move_sprite(30, 0, 5 << 3);
    set_sprite_tile(31, oga_splash_movable_map[splashscreen_line_width * 5 +
                                               splashscreen_a_xoffset + 5]);
    move_sprite(31, 5 << 3, 5 << 3);

    // move A over G
    for (i = 8; i < 32; ++i) {
        scroll_sprite(i, 9 << 3, 6 << 3);
    }
    SHOW_SPRITES;

    // move slowly in 8px steps
    for (j = 0; j < 6; ++j) {
        delay(100);
        // A
        for (i = 8; i < 32; ++i) {
            scroll_sprite(i, 8, 0);
        }
        // O
        memcpy(background, oga_splash_map_og, 360);
        move_head(4 - j);
        set_bkg_tiles(0, 0, 20, 18, background);
        for (i = 0; i < 8; ++i) {
            scroll_sprite(i, -8, 0);
        }
    }

    memcpy(background, oga_splash_map, 360);
    // remove text
    for (i = 2; i < 4; ++i) {
        for (j = 0; j < 20; ++j) {
            background[i * 20 + j] = 0x00; // blank them
        }
    }
    for (i = 11; i < 14; ++i) {
        for (j = 0; j < 20; ++j) {
            background[i * 20 + j] = 0x00; // blank them
        }
    }
    HIDE_SPRITES;
    set_bkg_tiles(0, 0, 20, 18, background);
    // clean up sprites
    for (i = 0; i < 32; ++i) {
        move_sprite(i, 0, 0);
        set_sprite_prop(i, 0x00);
    }
    delay(1000);
    set_bkg_tiles(0, 0, 20, 18, oga_splash_map);
    set_sprite_tile(0, oga_splash_movable_map[splashscreen_line_width + 2]);
    move_sprite(0, 3 << 3, 9 << 3);
    // set_sprite_prop(0, S_PALETTE);
    set_sprite_tile(1, oga_splash_movable_map[splashscreen_line_width + 3]);
    move_sprite(1, 4 << 3, 9 << 3);
    set_sprite_prop(1, S_PALETTE);
    set_sprite_tile(2, oga_splash_movable_map[splashscreen_line_width + 4]);
    move_sprite(2, 4 << 3, 10 << 3);
    set_sprite_prop(2, S_PALETTE);
    // modify main palette
    // dark grey and white get switched
    // also light grey becomes dark grey
    OBP0_REG = 0xCA; // 11001010
    SHOW_SPRITES;
    bling();
    delay(100);
    HIDE_SPRITES;
    // clean up again
    for (i = 0; i < 4; i++) {
        move_sprite(i, 0, 0);
        set_sprite_prop(i, 0x00);
    }
    // reset main palette
    OBP0_REG = 0xE4; // 11100100
    delay(1000);
    // fadeout
    for (i = 0; i < 4; i++) {
        switch (i) {
        case 0:
            BGP_REG = 0xE4;
            break;
        case 1:
            BGP_REG = 0xF9;
            break;
        case 2:
            BGP_REG = 0xFE;
            break;
        case 3:
            BGP_REG = 0xFF;
            break;
        }
        delay(100);
    };
}
