#include "menu.h"
#include "island_joy_16.h"
#include "pix/great_burst_win_map_clear.c"
#include "text_en.c"

static const UINT16 bg_palette[] = {
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

static const UINT16 bg_metal_palette[] = {IJ16_YELLOW, IJ16_BEIGE, IJ16_GREY,
                                          IJ16_BLACK,  IJ16_BLACK, IJ16_BLACK,
                                          IJ16_BLACK};

static const UINT16 fg_palette[] = {
    // metal
    0, IJ16_BEIGE, IJ16_GREY, IJ16_BLACK,
    // metal reflection
    0, IJ16_BEIGE, IJ16_GREY, IJ16_YELLOW,
    // green paddle
    0, IJ16_LIGHT_GREEN, IJ16_DARK_GREEN, IJ16_BLACK,
    // fire
    0, IJ16_ORANGE, IJ16_DARK_PINK, IJ16_BLACK};

UINT8 buffer[16];

void write_line(UINT8 x, UINT8 y, UINT8 length, char *str) {
    UINT8 i;
    for (i = 0; i < 16; i++) {
        buffer[i] = font_space;
    }
    for (i = 0; i < length; i++) {
        // strings end with a nullbyte
        if (str[i] == '\0') {
            break;
        }
        if (str[i] > 0x20 && str[i] < 0x60) {
            buffer[i] = font_start + (str[i] - font_offset);
        } else if (str[i] > 0x60 && str[i] < 0x7B) {
            // we don't have lower case in our font
            // shift to upper case
            buffer[i] = font_start + (str[i] - font_to_uppercase);
        } else {
            // everything else, including space, becomes a space
            buffer[i] = font_space;
        }
    }
    set_win_tiles(x, y, length, 1, buffer);
}

void write_text(UINT8 x, UINT8 y, UINT8 width, UINT8 height, UINT8 offset,
                char *str, UINT8 length) {
    UINT8 i, tmp_buffer, j = 0, row = 0;
    // skip lines
    for (i = 0; i < length && offset > 0; ++i)
        if (str[i] == '\n')
            --offset;
    // don't reset character index
    for (; i < length && row < height; ++i) {
        // clear buffer
        if (j == 0) {
            for (j = 0; j < 16; ++j) {
                buffer[j] = font_space;
            }
            j = 0;
        }
        // strings end with a nullbyte
        if (str[i] == '\0') {
            break;
        }
        // those characters are very special
        if (str[i] == '\a') {
            // alternative character mode
            if ((i + 1) < length) {
                ++i;
                // map rows all to first font row
                tmp_buffer =
                    font_start + ((str[i] & 0x0F) + 0x10 - font_offset);
                if (str[i] == '"' || str[i] == 'u' || str[i] == '\'' ||
                    str[i] == 'I' || str[i] == ';' || str[i] == '\\' ||
                    str[i] == '-' || str[i] == '^') {
                    // write above next character
                    set_win_tiles(x + j, y + row - 1, 1, 1, &tmp_buffer);
                } else {
                    // add character
                    if (j < 16)
                        buffer[j++] = tmp_buffer;
                }
            }
            continue;
        } else if (str[i] == '\n') {
            set_win_tiles(x, y + row, width, 1, buffer);
            j = 0;
            ++row;
            continue;
        }
        // those characters get printed
        if (str[i] == '.' && (i + 1) < length && str[i + 1] == '0') {
            // there is a special .0 character
            ++i;
            tmp_buffer = font_start + ((('0' & 0x0F) + 0x10) - font_offset);
        } else if (str[i] > 0x20 && str[i] < 0x60) {
            // print regular characters
            tmp_buffer = font_start + (str[i] - font_offset);
        } else if (str[i] > 0x60 && str[i] < 0x7B) {
            // we don't have lower case in our font
            // shift to upper case
            tmp_buffer = font_start + (str[i] - font_to_uppercase);
        } else {
            buffer[i] = font_space;
            switch (str[i]) {
            case '\t':
                if (j < 16)
                    buffer[j++] = font_space;
                if (j < 16)
                    buffer[j++] = font_space;
                if (j < 16)
                    buffer[j++] = font_space;
            default: // space
                tmp_buffer = font_space;
            }
        }
        // avoid overflow
        if (j < 16)
            buffer[j++] = tmp_buffer;
    }
    // ended with a \0 and we just printed at \n
    if (j != 0) {
        set_win_tiles(x, y + row, width, 1, buffer);
        ++row;
    }
    if (row < height) {
        // clear buffer
        for (i = 0; i < 16; ++i) {
            buffer[i] = font_space;
        }
        for (; row < height; row++) {
            set_win_tiles(x, y + row, width, 1, buffer);
        }
    }
}

void draw_menu(UINT8 mode) {
    set_win_tiles(0, 0, 20, 18, great_burst_win_map_clear);
    draw_stats();
    if (mode == 0) { // main menu
        write_text(7, 4, 12, 14, 0, text_main_menu, sizeof(text_main_menu));
    } else { // pause menu*/
        write_text(7, 4, 12, 14, 0, text_pause_menu, sizeof(text_pause_menu));
    }
    buffer[0] = 0x20;
    set_win_tiles(5, 4, 1, 1, buffer);
}

void help() {
    UINT8 i;
    write_text(4, 0, 15, 2, 0, text_help_title, sizeof(text_help_title));
    write_text(4, 2, 15, 16, 0, text_help, sizeof(text_help));
    for (i = 0; i < 16; ++i) {
        buffer[i] = 0x21;
    }
    set_win_tiles(19, 2, 1, 16, buffer);
    buffer[0] = 0x29;
    set_win_tiles(19, 3, 1, 1, buffer);
    for (i = 0; i < 40; ++i) {
        wait_vbl_done();
    }
}

void credits() {
    UINT8 i;
    write_text(4, 0, 16, 2, 0, text_credits_title, sizeof(text_credits_title));
    write_text(4, 2, 15, 16, 0, text_credits, sizeof(text_credits));
    for (i = 0; i < 16; ++i) {
        buffer[i] = 0x21;
    }
    // set_win_tiles(19, 2, 1, 16, buffer);
    // buffer[0] = 0x29;
    // set_win_tiles(19, 3, 1, 1, buffer);
    for (i = 0; i < 40; ++i) {
        wait_vbl_done();
    }
    write_text(4, 0, 16, 2, 0, text_credits_title, sizeof(text_credits_title));
    write_text(4, 2, 15, 16, 8, text_credits, sizeof(text_credits));
    for (i = 0; i < 16; ++i) {
        buffer[i] = 0x21;
    }
    for (i = 0; i < 40; ++i) {
        wait_vbl_done();
    }
    write_text(4, 0, 16, 2, 0, text_credits_title, sizeof(text_credits_title));
    write_text(4, 2, 15, 16, 16, text_credits, sizeof(text_credits));
    for (i = 0; i < 16; ++i) {
        buffer[i] = 0x21;
    }
    for (i = 0; i < 40; ++i) {
        wait_vbl_done();
    }
}

void slide_in() {
    UINT8 i;
    move_win(7 + (20 << 3) - (3 << 3), 0);
    // slide in
    for (i = 0; i < (20 << 3) - (3 << 3) - 3; i += 3) {
        scroll_win(-3, 0);
        wait_vbl_done();
    }
    move_win(7, 0);
}

void slide_out() {
    UINT8 i;
    move_win(7, 0);
    // slide in
    for (i = 0; i < (20 << 3) - (3 << 3) - 3; i += 3) {
        scroll_win(+3, 0);
        wait_vbl_done();
    }
    move_win(7 + (20 << 3) - (3 << 3), 0);
}

UINT8 menu(UINT8 mode) {
    UINT8 i;
    UINT8 selected = 0;
    UINT8 ret = 0;
    great_burst_init();
    for (i = 0; i < 16; ++i) {
        buffer[i] = 0x04;
    }
    // reset balls
    set_win_tiles(5, 4, 1, 16, buffer);
    draw_menu(mode);
    SHOW_WIN;
    if (mode == 0) {
        set_sprite_palette(0, 4, fg_palette);
        // first one will be defined in fade in
        set_bkg_palette(1, 7, bg_palette);
        SHOW_BKG;
        DISPLAY_ON;
        // fade in
        for (i = 3; i != -1; --i) {
            BGP_REG = (0xFFE4 >> (i << 1));
            set_bkg_palette(0, 1, bg_metal_palette + i);
            delay(100);
        }
    } else {
        slide_in();
    }
    for (i = 0; i < 5; ++i) {
        wait_vbl_done();
    }
    while (mode == 0 || ret == 0) {
        // help();
        // credits();
        // if (mode == 0 && joypad() == J_START) {
        //     great_burst_init();
        //     load_level(0, 0);
        //     slide_out();
        //     great_burst();
        //     draw_menu(mode);
        //     slide_in();
        // }
        if (mode == 1 && joypad() == J_START) {
            plonger(3);
            ret = 1;
        }
        switch (joypad()) {
        case J_UP:
            if (selected > 0) {
                plonger(5);
                buffer[0] = 0x04;
                set_win_tiles(5, 4 + (selected << 1), 1, 1, buffer);
                --selected;
                buffer[0] = 0x20;
                set_win_tiles(5, 4 + (selected << 1), 1, 1, buffer);
            }
            break;
        case J_DOWN:
            if ((mode == 0 && selected < 3) || (mode == 1 && selected < 2)) {
                plonger(5);
                buffer[0] = 0x04;
                set_win_tiles(5, 4 + (selected << 1), 1, 1, buffer);
                ++selected;
                buffer[0] = 0x20;
                set_win_tiles(5, 4 + (selected << 1), 1, 1, buffer);
            }
            break;
        case J_A:
        case J_B:
            plonger(6);
            if (mode == 0 && selected == 0) {
                great_burst_init();
                load_level(0, 0);
                slide_out();
                great_burst();
            } else if (mode == 0 && selected == 1) {
                great_burst_init();
                load_level(1, sys_time);
                slide_out();
                great_burst();
            } else if (mode == 0 && selected == 2) {
                help();
            } else if (mode == 0 && selected == 3) {
                credits();
            } else if (mode == 1 && selected == 0) {
                ret = 1;
            } else if (mode == 1 && selected == 1) {
                help();
            } else if (mode == 1 && selected == 2) {
                ret = 2;
            }
            draw_menu(mode);
            // remove ball from draw_menu
            buffer[0] = 0x04;
            set_win_tiles(5, 4, 1, 1, buffer);
            // draw ball on correct position
            buffer[0] = 0x20;
            set_win_tiles(5, 4 + (selected << 1), 1, 1, buffer);
            // after leaving a game, we have to slide back in
            if (mode == 0 && selected < 2)
                slide_in();
            break;
        }

        for (i = 0; i < 8; ++i) {
            wait_vbl_done();
        }
    };
    slide_out();
    return ret;
}
