#include <gb/gb.h>
#include <stdio.h>
#include <string.h>

#include "great_burst_level.c"

#include "pix/great_burst_bg_data.c"
#include "pix/great_burst_bg_map.c"
#include "pix/great_burst_bg_map_clear.c"

#include "pix/great_burst_fg_data.c"
#include "pix/great_burst_fg_map.c"

//field is 9 blocks wide and blocks high 10
//one UINT8 is 2 blocks => 2 rows = 9byte

UINT8 current_level[45];
const UINT8 map_block[] = {0x05, 0x11, 0x1D, 0x27, 0x06, 0x13, 0x1F, 0x29};

//directly modifies background variable
//always draws current_level
void draw_blocks(){
    UINT8 i;
    //load empty level background
    memcpy(background, great_burst_bg_map_clear, 360);
    //place blocks
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
    //update
    set_bkg_tiles(0,0,20,18, background);
}

void great_burst(){
    UINT8 changed = 0;
    UINT8 i;
    //set second color palette
    //dark grey and white get switched
    OBP1_REG = 0x26;//11000110

    //load sprite tileset
    set_sprite_data(0, 59, great_burst_fg_data);

    //draw ball
    set_sprite_tile(0,great_burst_fg_map[6*4]);
    move_sprite(0,0,0);
    set_sprite_prop(0, S_PALETTE);
    set_sprite_tile(1,great_burst_fg_map[6*4+1]);
    move_sprite(1,8,0);
    set_sprite_tile(2,great_burst_fg_map[6*5]);
    move_sprite(2,0,8);
    set_sprite_tile(3,great_burst_fg_map[6*5]+1);
    move_sprite(3,8,8);
    for(i = 0; i < 4; ++i){
        scroll_sprite(i, 5<<3, 15<<3);
    }

    //draw paddle
    //left and right side
    for(i = 0; i < 12; ++i){
        set_sprite_tile(4+i,great_burst_fg_map[i]);
        move_sprite(4+i,(i%6)<<3,(i/6)<<3);
    }
    set_sprite_prop(4+2, S_PALETTE);
    set_sprite_prop(4+3, S_PALETTE);

    //move to left
    for(i = 4; i < 16; ++i){
        scroll_sprite(i, 2<<3, 18<<3);
    }
    //middle parts, but this will stay hidden for now
    for(i = 0; i < 8; i+=2){
        set_sprite_tile(16+i,great_burst_fg_map[6*2]);
        set_sprite_prop(16+i, S_PALETTE);
        set_sprite_tile(17+i,great_burst_fg_map[6*3]);
        move_sprite(17+i,0,8);
    }

    //load background tileset
    set_bkg_data(0,58, great_burst_bg_data);
    //set level
    memcpy(current_level, great_burst_level[1], 45);
    //fill level background
    draw_blocks();
    SHOW_BKG;
    SHOW_SPRITES;
    BGP_REG = 0xE4;

}
