#include "menu.h"

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
    write_line(0, 0, 3, "133");
    write_line(2, 4, 1, "7");
    if (mode == 0) { // main menu
        write_text(7, 4, 12, 14, 0, text_main_menu, sizeof(text_main_menu));
    } else { // pause menu*/
        write_text(7, 4, 12, 14, 0, text_pause_menu, sizeof(text_pause_menu));
    }
    buffer[0] = 0x20;
    set_win_tiles(5, 4, 1, 1, buffer);
}

void help() {
    write_text(4, 0, 15, 2, 0, text_help_title, sizeof(text_help_title));
    write_text(4, 2, 15, 16, 0, text_help, sizeof(text_help));
    for (i = 0; i < 16; ++i) {
        buffer[i] = 0x21;
    }
    set_win_tiles(19, 2, 1, 16, buffer);
    buffer[0] = 0x29;
    set_win_tiles(19, 3, 1, 1, buffer);
    while (1)
        ;
}

void credits() {
    write_text(4, 0, 16, 2, 0, text_credits_title, sizeof(text_credits_title));
    write_text(4, 2, 15, 16, 16, text_credits, sizeof(text_credits));
    for (i = 0; i < 16; ++i) {
        buffer[i] = 0x21;
    }
    set_win_tiles(19, 2, 1, 16, buffer);
    buffer[0] = 0x29;
    set_win_tiles(19, 3, 1, 1, buffer);
    while (1)
        ;
}

void menu(UINT8 mode) {
    set_bkg_data(0, 163, great_burst_bg_data);
    draw_menu(mode);
    SHOW_BKG;
    SHOW_WIN;
    /*if(mode == 0){
        fade_in();
    }*/
    while (joypad() != J_START) {
        // help();
        credits();
        wait_vbl_done();
        wait_vbl_done();
        wait_vbl_done();
        wait_vbl_done();
        wait_vbl_done();
        wait_vbl_done();
        wait_vbl_done();
        wait_vbl_done();
    };
    HIDE_BKG;
    HIDE_WIN;
}
