#include <gb/gb.h>
#include <rand.h>
#include <stdio.h>
#include <string.h>

#include "great_burst_level.c"

#include "pix/great_burst_bg_data.c"
#include "pix/great_burst_bg_map.c"
#include "pix/great_burst_bg_map_clear.c"

#include "pix/great_burst_fg_data.c"
#include "pix/great_burst_fg_map.c"

// defines for sprite management
#define ball_start 0
#define ball_end 4
#define paddle_left_start 4
#define paddle_left_end 10
#define paddle_right_start 10
#define paddle_right_end 16
#define paddle_middle_start 16
#define paddle_middle_end 24
#define direction_max 24
#define direction_1st_quarter (direction_max >> 2)
#define direction_2nd_quarter (direction_max >> 1)
#define direction_3rd_quarter (direction_max >> 2) * 3
#define direction_4th_quarter direction_max

// field is 9 blocks wide and blocks high 10
// one UINT8 is 2 blocks => 2 rows = 9byte

typedef struct {
    UINT8 x;      // : 8;
    UINT8 y;      // : 8;
    UINT8 speed;  // : 2; // 0-2; 2 bit
    UINT8 locked; // : 1;    // 1 bit
    // 0 is up; 3 is 45 degree to the right; 6 is right; 12 is down...
    UINT8 direction; // : 5; // 0-23; 5bit
} Ball;

typedef struct {
    UINT8 position;
    UINT8 speed;
    UINT8 size;
} Paddle;

Ball ball = {0, 0, 1, 1, 1};
Paddle paddle = {0, 1, 6};

UINT8 current_level[45];
const UINT8 map_block[] = {0x05, 0x11, 0x1D, 0x27, 0x06, 0x13, 0x1F, 0x29};
const UINT8 map_shadow[] = {0x31, 0x32, 0x34, 0x15, 0x08, 0x33};

UINT8 random_block(UINT8 ran) {
    if (ran < 20) {
        return 0x01;
    } else if (ran < 35) {
        return 0x02;
    } else if (ran < 45) {
        return 0x03;
    } else if (ran < 55) {
        return 0x06;
    } else if (ran < 60) {
        return 0x05;
    } else if (ran < 65) {
        return 0x07;
    } else if (ran < 70) {
        return 0x04;
    }
    // this is way more than 30%
    return 0x00;
}

void random_level(UINT16 seed) {
    UINT8 i;
    initrand(seed);
    for (i = 0; i < 45; i++) {
        current_level[i] = random_block(rand() % 100);
        current_level[i] |= (random_block(rand() % 100)) << 4;
    }
}

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
    // last block in row does not have this shadow
    if ((2 + position) % 18 != 0) {
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

UINT8 mirror_direction(UINT8 direction, UINT8 horizontal) {
    // UINT8 ret = 0;
    if (horizontal) {
        return direction_max - direction;
    } else {
        return direction_max -
               ((direction + (direction_max >> 1)) % direction_max);
    }
}

// use middleparts at 16-24
void change_paddle_size(UINT8 size) {
    UINT8 i;
    // respect minimum size
    if (size < 6) {
        size = 6;
    }
    if (size > paddle.size) { // show more
        for (i = paddle.size - 6; i < size - 6; ++i) {
            scroll_sprite(paddle_middle_start + (i << 1), (3 + i) << 3,
                          18 << 3);
            scroll_sprite(paddle_middle_start + (i << 1) + 1, (3 + i) << 3,
                          18 << 3);
        }
        // center ball
        if (ball.locked) {
            for (i = ball_start; i < ball_end; ++i) {
                scroll_sprite(i, (size - paddle.size) << 2, 0);
            }
            ball.x += (size - paddle.size) << 2;
        }
    } else { // show less
        for (i = size - 6; i < paddle.size - 6; ++i) {
            move_sprite(paddle_middle_start + (i << 1), 16, 0);
            move_sprite(paddle_middle_start + (i << 1) + 1, 16, 8);
        }
        // center ball
        if (ball.locked) {
            for (i = ball_start; i < ball_end; ++i) {
                scroll_sprite(i, ((size - paddle.size) << 2), 0);
            }
            ball.x += (size - paddle.size) << 2;
        }
    }
    // move right end;
    for (i = paddle_right_start; i < paddle_right_end; ++i) {
        scroll_sprite(i, (size - paddle.size) << 3, 0);
    }
    paddle.size = size;
}

void move_paddle(UINT8 by) {
    UINT8 i;
    paddle.position += by;
    for (i = paddle_left_start; i < paddle_right_end + ((paddle.size - 6) << 1);
         ++i) {
        scroll_sprite(i, by, 0);
    }
    if (ball.locked) {
        for (i = ball_start; i < ball_end; ++i) {
            scroll_sprite(i, by, 0);
        }
        ball.x += by;
    }
}

// sound effect generator
void plonger(UINT8 note) {
    NR10_REG = 0x13; // arpeggio | 3
    NR11_REG = 0x50; // 50% duty
    NR12_REG = 0xF7; // volume envelope
    switch (note) {
    case 0:              // break
        NR13_REG = 0x7F; // lsb
        NR14_REG = 0xC2; // msb
        break;
    case 1:              // paddle
        NR13_REG = 0x00; // lsb
        NR14_REG = 0xC3; // msb
        break;
    case 3:              // shoot
        NR10_REG = 0x14; // arpeggio | 4
        NR13_REG = 0x70; // lsb
        NR14_REG = 0xC4; // msb
    case 2:
    default:             // wall
        NR13_REG = 0x00; // lsb
        NR14_REG = 0xC4; // msb
    }
}

void fade_in(){
    UINT8 i;
    // fadein
    for (i = 0; i < 4; i++) {
        switch (i) {
        case 0:
            BGP_REG = 0xFF;
            break;
        case 1:
            BGP_REG = 0xFE;
            break;
        case 2:
            BGP_REG = 0xF9;
            break;
        case 3:
            BGP_REG = 0xE4;
            break;
        }
        delay(100);
    };
}

void great_burst() {
    UINT8 changed = 0;
    UINT8 i;

    INT8 tmp_x = 0;
    INT8 tmp_y = 0;

    // set second color palette
    // dark grey and white get switched
    OBP1_REG = 0x26; // 11000110

    // load sprite tileset
    set_sprite_data(0, 59, great_burst_fg_data);

    // draw ball
    set_sprite_tile(ball_start, great_burst_fg_map[6 * 4]);
    move_sprite(ball_start, 0, 0);
    set_sprite_prop(ball_start + 0, S_PALETTE);
    set_sprite_tile(ball_start + 1, great_burst_fg_map[6 * 4 + 1]);
    move_sprite(ball_start + 1, 8, 0);
    set_sprite_tile(ball_start + 2, great_burst_fg_map[6 * 5]);
    move_sprite(ball_start + 2, 0, 8);
    set_sprite_tile(ball_start + 3, great_burst_fg_map[6 * 5] + 1);
    move_sprite(ball_start + 3, 8, 8);
    // place on 0,0
    for (i = ball_start; i < ball_end; ++i) {
        scroll_sprite(i, 16, 18 << 3);
    }
    // place 16, 16
    for (i = ball_start; i < ball_end; ++i) {
        scroll_sprite(i, 16, -16);
    }
    ball.x = ball.y = 16;

    // draw paddle
    // left side
    for (i = 0; i < 6; ++i) {
        set_sprite_tile(paddle_left_start + i,
                        great_burst_fg_map[i + (i >= 3 ? 3 : 0)]);
        move_sprite(paddle_left_start + i, (i % 3) << 3, (i >= 3) << 3);
    }
    // right side
    for (i = 0; i < 6; ++i) {
        set_sprite_tile(paddle_right_start + i,
                        great_burst_fg_map[i + (i >= 3 ? 6 : 3)]);
        move_sprite(paddle_right_start + i, ((i % 3) + 3) << 3, (i >= 3) << 3);
    }
    // use different palette for shiny parts
    set_sprite_prop(paddle_left_start + 2, S_PALETTE);
    set_sprite_prop(paddle_right_start + 0, S_PALETTE);

    // move to left
    for (i = paddle_left_start; i < paddle_right_end; ++i) {
        scroll_sprite(i, 16, 18 << 3);
    }
    // middle parts, but this will stay hidden for now
    for (i = paddle_middle_start; i < paddle_middle_end; i += 2) {
        set_sprite_tile(i, great_burst_fg_map[6 * 2]);
        set_sprite_prop(i, S_PALETTE);
        set_sprite_tile(i + 1, great_burst_fg_map[6 * 3]);
        move_sprite(i + 1, 0, 8);
    }
    // move them to x start position
    for (i = paddle_middle_start; i < paddle_middle_end; ++i) {
        scroll_sprite(i, 16, 0);
    }

    // load background tileset
    set_bkg_data(0, 59, great_burst_bg_data);
    // set level
    // memcpy(current_level, great_burst_level[0], 45);
    random_level(42);
    // fill level background
    draw_blocks();
    SHOW_BKG;
    fade_in();
    SHOW_SPRITES;
    while (1) {
        if (!ball.locked) {
            // move ball
            tmp_x = 0;
            tmp_y = 0;
            if (ball.direction < direction_2nd_quarter) {
                tmp_y = (ball.direction - direction_1st_quarter) * ball.speed;
            } else if (ball.direction < direction_4th_quarter) {
                tmp_y = (direction_3rd_quarter - ball.direction) * ball.speed;
            }
            if (ball.direction < direction_1st_quarter) {
                tmp_x = ball.direction * ball.speed;
            } else if (ball.direction < direction_2nd_quarter) {
                tmp_x = (direction_2nd_quarter - ball.direction) * ball.speed;
            } else if (ball.direction < direction_3rd_quarter) {
                tmp_x = (ball.direction - direction_2nd_quarter) * -ball.speed;
            } else if (ball.direction < direction_4th_quarter) {
                tmp_x = (direction_4th_quarter - ball.direction) * -ball.speed;
            }
            // 6 double blocks wide - ball width
            if (tmp_x + ball.x > ((18 - 2) << 3)) {
                // tmp_x = ((18-2)<<3) - ball.x;
                tmp_x = 0;
                ball.direction = mirror_direction(ball.direction, 1);
                plonger(2);
            } else if ((ball.direction > direction_2nd_quarter) &&
                       ball.x < -tmp_x) {
                tmp_x = ball.x;
                ball.direction = mirror_direction(ball.direction, 1);
                plonger(2);
            }
            // 17 double blocks high - ball height
            if (ball.y - tmp_y > ((17 - 2) << 3)) {
                // mirror ball path partly
                // tmp_y = -(((17-2)<<3) - ball.y);
                tmp_y = 0;
                // change future ball direction
                ball.direction = mirror_direction(ball.direction, 0);
                plonger(2);
            } else if ((ball.direction > 6 && ball.direction <= 18) &&
                       ball.y < tmp_y) {
                tmp_y = -ball.y;
                ball.direction = mirror_direction(ball.direction, 0);
                plonger(2);
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
            ball.direction = mirror_direction(ball.direction, 1);
        }
        if (joypad() == (J_A | J_UP)) {
            ball.direction = mirror_direction(ball.direction, 0);
        }

        // unlock ball
        if (ball.locked && joypad() & J_A) {
            ball.locked = 0;
            plonger(3);
        }
        // control paddle
        paddle.speed = 2;
        if (joypad() & J_B) {
            paddle.speed = 4;
        }
        // only check directions
        switch (joypad() & 0x0F) {
        case J_RIGHT:
            if (paddle.position + paddle.speed >
                ((18 - paddle.size) << 3)) { //*8*2
                paddle.speed = ((18 - paddle.size) << 3) - paddle.position;
            }
            move_paddle(paddle.speed);
            break;
        case J_LEFT:
            if (paddle.position < paddle.speed) {
                paddle.speed = paddle.position;
            }
            move_paddle(-paddle.speed);
            break;
        case J_UP:
            move_paddle(-paddle.position);
            break;
        case J_DOWN:
            // jump to the right
            if (!(joypad() & J_B)) { // jump to half paddle
                move_paddle(((18 - paddle.size) << 3) - paddle.position);
            } else { // jump full
                move_paddle(((18 - paddle.size) << 2) - paddle.position);
            }
            break;
        }
        for (i = 0; i < 3; ++i) {
            wait_vbl_done();
        }
    }
}
