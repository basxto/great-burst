#include <gb/gb.h>
#include <stdio.h>
#include <string.h>

#include "great_burst_level.c"

#include "pix/great_burst_bg_data.c"
#include "pix/great_burst_bg_map.c"
#include "pix/great_burst_bg_map_clear.c"

#include "pix/great_burst_fg_data.c"
#include "pix/great_burst_fg_map.c"

// field is 9 blocks wide and blocks high 10
// one UINT8 is 2 blocks => 2 rows = 9byte

typedef struct {
    UINT8 x;// : 8;
    UINT8 y;// : 8;
    UINT8 speed;// : 2; // 0-2; 2 bit
    UINT8 locked;// : 1;    // 1 bit
    // 0 is up; 3 is 45 degree to the right; 6 is right; 12 is down...
    UINT8 direction;// : 5; // 0-23; 5bit
} Ball;

UINT8 current_level[45];
const UINT8 map_block[] = {0x05, 0x11, 0x1D, 0x27, 0x06, 0x13, 0x1F, 0x29};
const UINT8 map_shadow[] = {0x31, 0x32, 0x34, 0x15, 0x08, 0x33};

void draw_block(UINT8 position, UINT8 block) {
    const UINT8 pos0 = 20 * ((0 + position) / 18) + (0 + position) % 18;
    const UINT8 pos1 = 20 * ((1 + position) / 18) + (1 + position) % 18;
    const UINT8 pos2 = 20 * ((2 + position) / 18) + (2 + position) % 18;
    // row + offset (1 row + 1 column=21) + column
    background[pos0 + 21] = map_block[block];
    background[pos1 + 21] = map_block[block] + 1;
    // draw shadow
    if (background[pos0 + 41] == map_shadow[2]) {
        // if there is a shadow edge, use full shadow
        background[pos0 + 41] = map_shadow[1];
    } else {
        background[pos0 + 41] = map_shadow[0];
    }
    background[pos1 + 41] = map_shadow[1];
    if ((2 + position) % 18 !=
        0) { // last block in row does not have this shadow
        if (background[pos2 + 21] == map_shadow[2]) {
            // if there is a shadow edge, use full shadow
            background[pos2 + 21] = map_shadow[3];
        } else if (background[pos2 + 21] == map_block[0]) {
            // if there is shadow above use double shadow
            background[pos2 + 21] = map_shadow[4];
        } else {
            background[pos2 + 21] = map_shadow[5];
        }
        background[pos2 + 41] = map_shadow[2];
    }
}

// directly modifies background variable
// always draws current_level
void draw_blocks() {
    UINT8 i;
    // load empty level background
    memcpy(background, great_burst_bg_map_clear, 360);
    // place blocks
    for (i = 0; i < 45; ++i) {
        // 20 per line 1 offset
        // first block is border
        if (((current_level[i] >> 4) & 0x07) != 0) {
            draw_block((i << 2), (current_level[i] >> 4) & 0x07);
        }
        if ((current_level[i] & 0x07) != 0) {
            draw_block((i << 2) + 2, current_level[i] & 0x07);
        }
    }
    // update
    set_bkg_tiles(0, 0, 20, 18, background);
}

UINT8 mirrorDirection(UINT8 direction, UINT8 horizontal) {
    // UINT8 ret = 0;
    if (horizontal) {
        return 24 - direction;
    } else {
        return 24 - ((direction + 12) % 24);
    }
}

void great_burst() {
    UINT8 changed = 0;
    UINT8 i;
    Ball ball = {0, 0, 1, 1, 1};
    UINT8 paddle = 0;
    UINT8 speed = 1;

    INT8 tmp_x = 0;
    INT8 tmp_y = 0;

    // set second color palette
    // dark grey and white get switched
    OBP1_REG = 0x26; // 11000110

    // load sprite tileset
    set_sprite_data(0, 59, great_burst_fg_data);

    // draw ball
    set_sprite_tile(0, great_burst_fg_map[6 * 4]);
    move_sprite(0, 0, 0);
    set_sprite_prop(0, S_PALETTE);
    set_sprite_tile(1, great_burst_fg_map[6 * 4 + 1]);
    move_sprite(1, 8, 0);
    set_sprite_tile(2, great_burst_fg_map[6 * 5]);
    move_sprite(2, 0, 8);
    set_sprite_tile(3, great_burst_fg_map[6 * 5] + 1);
    move_sprite(3, 8, 8);
    // place on 0,0
    for (i = 0; i < 4; ++i) {
        scroll_sprite(i, 2 << 3, 18 << 3);
    }
    // place 16, 16
    for (i = 0; i < 4; ++i) {
        scroll_sprite(i, 16, -16);
    }
    ball.x = ball.y = 16;

    // draw paddle
    // left and right side
    for (i = 0; i < 12; ++i) {
        set_sprite_tile(4 + i, great_burst_fg_map[i]);
        move_sprite(4 + i, (i % 6) << 3, (i / 6) << 3);
    }
    set_sprite_prop(4 + 2, S_PALETTE);
    set_sprite_prop(4 + 3, S_PALETTE);

    // move to left
    for (i = 4; i < 16; ++i) {
        scroll_sprite(i, 2 << 3, 18 << 3);
    }
    // middle parts, but this will stay hidden for now
    for (i = 0; i < 8; i += 2) {
        set_sprite_tile(16 + i, great_burst_fg_map[6 * 2]);
        set_sprite_prop(16 + i, S_PALETTE);
        set_sprite_tile(17 + i, great_burst_fg_map[6 * 3]);
        move_sprite(17 + i, 0, 8);
    }

    // load background tileset
    set_bkg_data(0, 59, great_burst_bg_data);
    // set level
    memcpy(current_level, great_burst_level[0], 45);
    // fill level background
    draw_blocks();
    SHOW_BKG;
    SHOW_SPRITES;
    BGP_REG = 0xE4;
    while (1) {
        if (!ball.locked) {
            // move ball
            tmp_x = 0;
            tmp_y = 0;
            if (ball.direction < 12) {
                tmp_y = (ball.direction - 6) * ball.speed;
            } else if (ball.direction < 24) {
                tmp_y = (18 - ball.direction) * ball.speed;
            }
            if (ball.direction < 6) {
                tmp_x = ball.direction * ball.speed;
            } else if (ball.direction < 12) {
                tmp_x = (12 - ball.direction) * ball.speed;
            } else if (ball.direction < 18) {
                tmp_x = (ball.direction - 12) * -ball.speed;
            } else if (ball.direction < 24) {
                tmp_x = (24 - ball.direction) * -ball.speed;
            }
            // 6 double blocks wide - ball width
            if (tmp_x + ball.x > ((18 - 2) << 3)) {
                // tmp_x = ((18-2)<<3) - ball.x;
                tmp_x = 0;
                ball.direction = mirrorDirection(ball.direction, 1);
            } else if ((ball.direction > 12) && ball.x < -tmp_x) {
                tmp_x = ball.x;
                ball.direction = mirrorDirection(ball.direction, 1);
            }
            // 17 double blocks high - ball height
            if (ball.y - tmp_y > ((17 - 2) << 3)) {
                // mirror ball path partly
                // tmp_y = -(((17-2)<<3) - ball.y);
                tmp_y = 0;
                // change future ball direction
                ball.direction = mirrorDirection(ball.direction, 0);
            } else if ((ball.direction > 6 && ball.direction <= 18) &&
                       ball.y < tmp_y) {
                tmp_y = -ball.y;
                ball.direction = mirrorDirection(ball.direction, 0);
            }
            // actually move ball
            for (i = 0; i < 4; ++i) {
                scroll_sprite(i, tmp_x, tmp_y);
            }
            // move theoretic ball
            ball.x += tmp_x;
            ball.y -= tmp_y;
        }
        // cheat & debug codes
        if (joypad() == (J_A | J_DOWN)) {
            ball.speed = (ball.speed + 2) % 8;
        }
        if (joypad() == (J_A | J_LEFT)) {
            ball.direction = mirrorDirection(ball.direction, 1);
        }
        if (joypad() == (J_A | J_UP)) {
            ball.direction = mirrorDirection(ball.direction, 0);
        }

        //unlock ball
        if (joypad() & J_A) {
            ball.locked = 0;
        }
        // control paddle
        speed = 2;
        if (joypad() & J_B) {
            speed = 4;
        }
        // only check directions
        switch (joypad() & 0x0F) {
        case J_RIGHT:
            if (paddle + speed > ((9 - 3) << 4)) { //*8*2
                speed = ((9 - 3) << 4) - paddle;
            }
            paddle += speed;
            for (i = 4; i < 16; ++i) {
                scroll_sprite(i, speed, 0);
            }
            if(ball.locked){
                for (i = 0; i < 4; ++i) {
                    scroll_sprite(i, speed, 0);
                }
                ball.x += speed;
            }
            break;
        case J_LEFT:
            if (paddle < speed) {
                speed = paddle;
            }
            paddle -= speed;
            for (i = 4; i < 16; ++i) {
                scroll_sprite(i, -speed, 0);
            }
            if(ball.locked){
                for (i = 0; i < 4; ++i) {
                    scroll_sprite(i, -speed, 0);
                }
                ball.x -= speed;
            }
            break;
        case J_UP:
            // jump to the left
            for (i = 4; i < 16; ++i) {
                scroll_sprite(i, -paddle, 0);
            }
            if(ball.locked){
                for (i = 0; i < 4; ++i) {
                    scroll_sprite(i, -paddle, 0);
                }
                ball.x -= paddle;
            }
            paddle = 0;
            break;
        case J_DOWN:
            // jump to the right
            if (!(joypad() & J_B)) { // jump to half paddle
                for (i = 4; i < 16; ++i) {
                    scroll_sprite(i, ((9 - 3) << 4) - paddle, 0);
                }
                if(ball.locked){
                    for (i = 0; i < 4; ++i) {
                        scroll_sprite(i, ((9 - 3) << 4) - paddle, 0);
                    }
                    ball.x += ((9 - 3) << 4) - paddle;
                }
                paddle = ((9 - 3) << 4);
            } else { // jump to half paddle (additional /2)
                for (i = 4; i < 16; ++i) {
                    scroll_sprite(i, ((9 - 3) << 3) - paddle, 0);
                }
                if(ball.locked){
                    for (i = 0; i < 4; ++i) {
                        scroll_sprite(i, ((9 - 3) << 3) - paddle, 0);
                    }
                    ball.x += ((9 - 3) << 3) - paddle;
                }
                paddle = ((9 - 3) << 3);
            }
            break;
        }
        for (i = 0; i < 3; ++i) {
            wait_vbl_done();
        }
    }
}
