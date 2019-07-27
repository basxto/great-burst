#include "pix/great_burst_win_data.c"
#include "pix/great_burst_win_map_clear.c"
#include "text_en.c"

UINT8 buffer[16];

#define font_start 0x1D

void write_line(UINT8 x, UINT8 y, UINT8 length, char *str) {
    UINT8 i;
    for (i = 0; i < 16; i++) {
        buffer[i] = 0x02;
    }
    for (i = 0; i < length; i++) {
        // strings end with a nullbyte
        if (str[i] == '\0') {
            break;
        }
        if (str[i] > 0x20 && str[i] < 0x60) {
            buffer[i] = 0x1D + (str[i] - 0x21);
        } else if (str[i] > 0x60 && str[i] < 0x7B) {
            // we don't have lower case in our font
            // shift to upper case
            buffer[i] = 0x1D + (str[i] - 0x41);
        } else {
            // everything else, including space, becomes a space
            buffer[i] = 0x02;
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
                buffer[j] = 0x02;
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
            if ((i + 1) < length && str[i + 1] == '^') {
                ++i;
                tmp_buffer = font_start + (str[i] - 0x21);
                set_win_tiles(x + j, y + row - 1, 1, 1, &tmp_buffer);
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
            tmp_buffer = font_start + ('0' - 0x21);
        } else if (str[i] > 0x20 && str[i] < 0x60) {
            // print regular characters
            tmp_buffer = font_start + (str[i] - 0x21);
        } else if (str[i] > 0x60 && str[i] < 0x7B) {
            // we don't have lower case in our font
            // shift to upper case
            tmp_buffer = font_start + (str[i] - 0x41);
        } else {
            buffer[i] = 0x02;
            switch (str[i]) {
            case '\t':
                if (j < 16)
                    buffer[j++] = 0x02;
                if (j < 16)
                    buffer[j++] = 0x02;
                if (j < 16)
                    buffer[j++] = 0x02;
            default: // space
                tmp_buffer = 0x02;
            }
        }
        // avoid overflow
        if (j < 16)
            buffer[j++] = tmp_buffer;
    }
    // ended with a \0 and we just printed at \n
    if (j != 0) {
        set_win_tiles(x, y + row, width, 1, buffer);
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
    buffer[0] = 0x1A;
    set_win_tiles(5, 4, 1, 1, buffer);
}

void help() {
    write_text(4, 0, 15, 2, 0, text_help_title, sizeof(text_help_title));
    write_text(4, 2, 15, 16, 0, text_help, sizeof(text_help));
    for (i = 0; i < 16; ++i) {
        buffer[i] = 0x5C;
    }
    set_win_tiles(19, 2, 1, 16, buffer);
    buffer[0] = 0x5D;
    set_win_tiles(19, 3, 1, 1, buffer);
    while (1)
        ;
}

void credits() {
    write_text(4, 0, 15, 2, 0, text_credits_title, sizeof(text_credits_title));
    write_text(4, 2, 15, 16, 0, text_credits, sizeof(text_credits));
    for (i = 0; i < 16; ++i) {
        buffer[i] = 0x5C;
    }
    set_win_tiles(19, 2, 1, 16, buffer);
    buffer[0] = 0x5D;
    set_win_tiles(19, 3, 1, 1, buffer);
    while (1)
        ;
}

void menu(UINT8 mode) {

    set_win_data(0, 150, great_burst_win_data);
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
