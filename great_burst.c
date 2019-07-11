#include <gb/gb.h>
#include <stdio.h>
#include <string.h>

#include "great_burst_level.c"

#include "pix/great_burst_bg_data.c"
#include "pix/great_burst_bg_map.c"
#include "pix/great_burst_bg_map_clear.c"

//field is 9 blocks wide and blocks high 10
//one UINT8 is 2 blocks => 2 rows = 9byte

UINT8 current_level[45];
const UINT8 map_block[] = {0x05, 0x11, 0x1D, 0x27, 0x06, 0x13, 0x1F, 0x29};

//directly modifies background variable
//always draws current_level
void draw_blocks(){
    UINT8 i;
    for(i = 0; i < 45; ++i){
        //20 per line 1 offset
        //first block is border
        
        if(((current_level[i]>>4)&0x07) != 0){
            //row + offset (1 row + 1 column=21) + column
            background[20 * ((0 + (i<<2)) / 18) + 21 + (0 + (i<<2)) % 18] = map_block[(current_level[i]>>4)&0x07];
            background[20 * ((1 + (i<<2)) / 18) + 21 + (1 + (i<<2)) % 18] = map_block[(current_level[i]>>4)&0x07]+1;
        }
        if((current_level[i]&0x07)  != 0){
            background[20 * ((2 + (i<<2)) / 18) + 21 + (2 + (i<<2)) % 18] = map_block[current_level[i]&0x07];
            background[20 * ((3 + (i<<2)) / 18) + 21 + (3 + (i<<2)) % 18] = map_block[current_level[i]&0x07]+1;
        }
    }
}

void great_burst(){
    UINT8 changed = 0;
    set_bkg_data(0,58, great_burst_bg_data);
    //load empty level background
    memcpy(background, great_burst_bg_map_clear, 360);
    //set level
    memcpy(current_level, great_burst_level[1], 45);
    //fill level background
    draw_blocks();
    set_bkg_tiles(0,0,20,18, background);
    SHOW_BKG;
    BGP_REG = 0xE4;
    
}
