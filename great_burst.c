#include "great_burst.h"

#include "background.c"

#include "great_burst_level.c"

#include "pix/great_burst_bg_data.c"
#include "pix/great_burst_bg_map.c"
#include "pix/great_burst_bg_map_clear.c"

#include "pix/great_burst_fg_data.c"
#include "pix/great_burst_fg_map.c"

Ball ball = {0, 0, 1, 1, 1};
Paddle paddle = {0, 1, 6};

UINT8 current_level[((field_width * field_height) >> 1)];
const UINT8 map_block[] = {0x04, 0x0D, 0x13, 0x17, 0x06, 0x0F, 0x15, 0x19};
const UINT8 map_shadow[] = {0x24, 0x25, 0x28, 0x11, 0x08, 0x27};
UINT8 i = 0;
UINT16 time;

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
    initrand(seed);
    for (i = 0; i < ((field_width * field_height) >> 1); i++) {
        current_level[i] = random_block(rand() % 100);
        current_level[i] |= (random_block(rand() % 100)) << 4;
    }
}

void one_block_level(UINT8 block) {
    for (i = 0; i < ((field_width * field_height) >> 1); i++) {
        current_level[i] = block & 0x0F;
        current_level[i] |= (block & 0x0F) << 4;
    }
}

// directly modifies background variable
// always draws current_level
void draw_blocks() {
    UINT8 position, block, pos0, pos1, pos2;
    // load empty level background
    memcpy(background, great_burst_bg_map_clear, 360);
    // place blocks
    // two blocks share one integer
    for (i = 0; i < (field_width * field_height); ++i) {
        // 20 per line 1 offset
        // first block is border
        if (i % 2 == 0) {
            position = ((i >> 1) << 2);
            block = (current_level[i >> 1] >> 4) & 0x07;
            if (block == 0) {
                continue;
            }
        }
        if (i % 2 == 1) {
            position = ((i >> 1) << 2) + 2;
            block = current_level[i >> 1] & 0x07;
            if (block == 0) {
                continue;
            }
        }
        pos0 = 20 * ((0 + position) / (field_width << 1)) +
               (0 + position) % (field_width << 1);
        pos1 = 20 * ((1 + position) / (field_width << 1)) +
               (1 + position) % (field_width << 1);
        pos2 = 20 * ((2 + position) / (field_width << 1)) +
               (2 + position) % (field_width << 1);
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
        if ((2 + position) % (field_width << 1) != 0) {
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
    // update
    set_bkg_tiles(0, 0, 20, 18, background);
}

// that offset of 21 makes no sense
UINT8 lock_ball() {
    for (i = ball_start; i < ball_end; ++i) {
        // scroll_sprite(i, (size - paddle.size) << 2, 0);
        scroll_sprite(i, -ball.x, ball.y - 16);
    }
    ball.y -= (ball.y - 16);
    // x is 0 now
    // 8/2 is <<2
    ball.x = paddle.position + ((paddle.size - 2) << 2);
    for (i = ball_start; i < ball_end; ++i) {
        scroll_sprite(i, ball.x, 0);
    }
    ball.locked = 1;
}

// only gets called for ball.y < 16 and ball.y can't be < 0
UINT8 collision_paddle(UINT8 x) {
    if ((x + 8) >= paddle.position &&
        x <= (paddle.position + (paddle.size << 3))) {
        return 1;
    }
    return 0;
}

// returns on collision
UINT8 collision_block(UINT8 position) {
    // caution: difference must not get negative (UINT8)
    // caution: 16*16 is an overflow (UINT8)
    // ball is globally known
    // 0,0 ball position is at 16, 18*8
    // first block is at 8+8, 16+8 because of srceen offset
    // ball radius is 8px (1<<3)

    // +8 since we need the center of the ball
    // y also needs +16 offscreen offset
    UINT8 x = ((position % field_width) << 4) - 8;
    UINT8 y = (18 << 3) - ((position / field_width) << 3) - 24;
    UINT8 diffx = 0;
    UINT8 diffy = 0;

    // is center of ball insized block
    if (x < ball.x && ball.x < (x + block_width) &&
        (y < (ball.y) && (ball.y) < (y + block_height))) {
        // plonger(0);
        // hit!!
        return 1;
    } else { // check distance of corners to ball center
        diffx = diff(ball.x, x);
        diffy = diff(ball.y, y);
        // with diffx=8 and diffy=8 we would have 8^2+8^2=128
        // this check prevents overflows that would appear at 16^2+0^2
        if (diffx <= 8 && diffy <= 8) {
            // r^2 >= diffx^2 + diffy^2
            if ((8 << 3) >= diffx * diffx + diffy * diffy) {
                return 1;
            }
        }
        diffy = diff(ball.y, y + block_height);
        if (diffx <= 8 && diffy <= 8) {
            // r^2 < diffx^2 + diffy^2
            if ((8 << 3) >= diffx * diffx + diffy * diffy) {
                return 1;
            }
        }
        diffx = diff(ball.x, x + block_width);
        if (diffx <= 8 && diffy <= 8) {
            if ((8 << 3) >= diffx * diffx + diffy * diffy) {
                return 1;
            }
        }
        diffy = diff(ball.y, y);
        if (diffx <= 8 && diffy <= 8) {
            if ((8 << 3) >= diffx * diffx + diffy * diffy) {
                return 1;
            }
        }
    }
    return 0;
}

// use middleparts at 16-24
void change_paddle_size(UINT8 size) {
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
    case 0:                                                   // break
        NR14_REG = 0xC0 | ((notes[0][note_fis] >> 8) & 0x07); // msb
        NR13_REG = notes[0][note_fis] & 0xFF;
        break;
    case 1:                                                   // paddle
        NR14_REG = 0xC0 | ((notes[0][note_gis] >> 8) & 0x07); // msb
        NR13_REG = notes[0][note_gis] & 0xFF;
        break;
    case 3:                                                 // shoot
        NR10_REG = 0x14;                                    // arpeggio | 4
        NR14_REG = 0xC0 | ((notes[0][note_h] >> 8) & 0x07); // msb
        NR13_REG = notes[0][note_h] & 0xFF;
        break;
    case 4:                                                 // die
        NR10_REG = 0x15;                                    // arpeggio | 5
        NR14_REG = 0xC0 | ((notes[1][note_d] >> 8) & 0x07); // msb
        NR13_REG = notes[4][note_d] & 0xFF;
        break;
    case 2:
    default:                                                // wall
        NR14_REG = 0xC0 | ((notes[0][note_h] >> 8) & 0x07); // msb
        NR13_REG = notes[0][note_h] & 0xFF;
    }
}

void fade_in() {
    BGP_REG = 0xFF;
    delay(100);
    BGP_REG = 0xFE;
    delay(100);
    BGP_REG = 0xF9;
    delay(100);
    BGP_REG = 0xE4;
    delay(100);
}

void great_burst_init() {
    // set second color palette
    // dark grey and white get switched
    OBP1_REG = 0x26; // 11000110

    // load sprite tileset
    set_sprite_data(0, 59, great_burst_fg_data);
    // load background tileset
    set_bkg_data(0, 163, great_burst_bg_data);
}

// load prebuilt or random level
void load_level(UINT8 random, UINT16 level) {
    if (random) {
        random_level(level);
    } else {
        memcpy(current_level, great_burst_level[level], 45);
    }
    // one_block_level(0x01);
    // fill level background
    draw_blocks();
}

void great_burst() {
    UINT8 changed = 0;
    UINT8 mask;

    INT8 tmp_x = 0;
    INT8 tmp_y = 0;

    // draw ball
    move_set_sprite(ball_start, great_burst_fg_map[6 * 4], 0, 0);
    set_sprite_prop(ball_start + 0, S_PALETTE);
    move_set_sprite(ball_start + 1, great_burst_fg_map[6 * 4 + 1], 8, 0);
    move_set_sprite(ball_start + 2, great_burst_fg_map[6 * 5], 0, 8);
    move_set_sprite(ball_start + 3, great_burst_fg_map[6 * 5] + 1, 8, 8);
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
        move_set_sprite(i + 1, great_burst_fg_map[6 * 3], 0, 8);
    }
    // move them to x start position
    for (i = paddle_middle_start; i < paddle_middle_end; ++i) {
        scroll_sprite(i, 16, 0);
    }

    SHOW_SPRITES;
    while (1) {
        changed = 0;
        time = sys_time;
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
            if (tmp_x + ball.x > (((field_width << 1) - 2) << 3)) {
                // tmp_x = ((18-2)<<3) - ball.x;
                tmp_x = 0;
                ball.direction = mirror_direction(ball.direction, 1);
                plonger(2);
            } else if ((ball.direction > direction_2nd_quarter) &&
                       ball.x < -tmp_x) {
                tmp_x = ball.x;
                changed |= 2;
                plonger(2);
            }
            // 17 double blocks high - ball height
            if (ball.y - tmp_y <= 16 && collision_paddle(ball.x + tmp_x)) {
                tmp_y = ball.y - 16;
                changed |= 1;
                plonger(1);
            } else if ((ball.direction > 6 &&
                        ball.direction <= direction_3rd_quarter) &&
                       ball.y < tmp_y) {
                plonger(4);
                changed |= 1;
                lock_ball();
            } else if (ball.y - tmp_y > ((17 - 2) << 3)) {
                // mirror ball path partly
                // tmp_y = -(((17-2)<<3) - ball.y);
                tmp_y = 0;
                // change future ball direction
                changed |= 1;
                plonger(2);
            }
            if (!ball.locked) {
                // actually move ball
                for (i = 0; i < 4; ++i) {
                    scroll_sprite(i, tmp_x, tmp_y);
                }
                // move theoretic ball
                ball.x += tmp_x;
                ball.y -= tmp_y;
            }
        }
        // cheat & debug codes
        if (joypad() == (J_A | J_DOWN)) {
            ball.speed = (ball.speed + 2) % 8;
        }
        if (joypad() == (J_A | J_LEFT)) {
            // ball.direction = mirror_direction(ball.direction, 1);
            changed |= 2;
        }
        if (joypad() == (J_A | J_UP)) {
            // ball.direction = mirror_direction(ball.direction, 0);
            changed |= 1;
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
                (((field_width << 1) - paddle.size) << 3)) { //*8*2
                paddle.speed =
                    (((field_width << 1) - paddle.size) << 3) - paddle.position;
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
                move_paddle((((field_width << 1) - paddle.size) << 3) -
                            paddle.position);
            } else { // jump full
                move_paddle((((field_width << 1) - paddle.size) << 2) -
                            paddle.position);
            }
            break;
        }
        tmp_x = (ball.x / block_width);
        tmp_y = (((18 << 3) - ball.y - 24) / block_height);
        // check for block collisions
        for (i = 0; i < (field_width * field_height); ++i) {
            // sort out empty double blocks
            if ((current_level[i >> 1]) == 0x00)
                continue;
            // only check blocks which surround the ball
            if (((i % field_width) < tmp_x) ||
                ((tmp_x + 1) < (i % field_width)) ||
                ((i / field_width) < tmp_y) ||
                ((tmp_y + 3) < (i / field_width)))
                continue;

            mask = (i & 0x01 ? 0x0F : 0xF0);
            if ((current_level[i >> 1] & mask)) {
                if (collision_block(i)) {
                    plonger(0);
                    switch (current_level[i >> 1] & mask) {
                    case 0x40: // wall
                    case 0x04:
                        plonger(2);
                        break;
                    case 0x20: // degrade
                    case 0x02:
                    case 0x30:
                    case 0x03:
                        plonger(2);
                        current_level[i >> 1] =
                            (current_level[i >> 1] & ~mask) |
                            (((current_level[i >> 1] & mask) - 1) & mask);
                        break;
                    default: // break
                        current_level[i >> 1] &= ~mask;
                    }
                    changed |= 1;
                }
            }
        }
        if (changed != 0) {
            draw_blocks();
            if (changed & 1)
                ball.direction = mirror_direction(ball.direction, 0);
            if (changed & 2)
                ball.direction = mirror_direction(ball.direction, 1);
        }
        if (joypad() == J_START) {
            HIDE_SPRITES;
            menu(1);
            SHOW_SPRITES;
            // basically button debouncing
            for (i = 0; i < 5; ++i) {
                wait_vbl_done();
            }
        }
        if ((sys_time - time) < 25) {
            for (i = 0; i < (25 - (sys_time - time)); ++i) {
                wait_vbl_done();
            }
        }
    }
}
