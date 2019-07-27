#include "pix/great_burst_win_data.c"
#include "pix/great_burst_win_map_clear.c"

UINT8 buffer[16];

void write_line(UINT8 x, UINT8 y, UINT8 length, char *str){
    for (i = 0; i < 16; i++){
        buffer[i] = 0x02;
    }
    for (i = 0; i < length; i++){
        // strings end with a nullbyte
        if(str[i] == '\00'){
            break;
        }
        if(str[i] > 0x20 && str[i] < 0x60){
            buffer[i] = 0x1D + (str[i]-0x21);
        } else if(str[i] > 0x60 && str[i] < 0x7B){
            // we don't have lower case in our font
            // shift to upper case
            buffer[i] = 0x1D + (str[i]-0x41);
        } else {
            // everything else, including space, becomes a space
            buffer[i] = 0x02;
        }
    }
    set_win_tiles(x, y, length, 1, buffer);

}

void menu(UINT8 mode){
    set_win_data(0, 150, great_burst_win_data);
    set_win_tiles(0, 0, 20, 18, great_burst_win_map_clear);
    write_line(8, 4, 9, "Play");
    write_line(8, 6, 9, "Help");
    write_line(8, 8, 9, "Random");
    write_line(8, 10, 9, "Options");
    write_line(8, 12, 9, "Credits");
    write_line(8, 14, 9, "Highscore");
    buffer[0] = 0x1A;
    set_win_tiles(6, 4, 1, 1, buffer);
    SHOW_BKG;
    SHOW_WIN;
    if(mode == 0){
        fade_in();
    }
    while(joypad()==0x00){
        wait_vbl_done();
    };
    HIDE_BKG;
    HIDE_WIN;
}
