#ifndef GREAT_BURST_H
#define GREAT_BURST_H

#include <gb/gb.h>
#include <rand.h>
#include <stdio.h>
#include <string.h>

#include "menu.h"
#include "sound.h"

// defines for sprite management
#define ball_start 0
#define ball_end 4
#define paddle_left_start 4
#define paddle_left_end 10
#define paddle_right_start 10
#define paddle_right_end 16
#define paddle_middle_start 16
#define paddle_middle_end 24

// TODO: 32 directions are probably easier to calculate
#define direction_max 24
#define direction_1st_quarter (direction_max >> 2)
#define direction_2nd_quarter (direction_max >> 1)
#define direction_3rd_quarter (direction_1st_quarter + direction_2nd_quarter)
#define direction_4th_quarter direction_max

#define block_width 16
#define block_height 8
// TODO: field width 8 would be faster to calculate with bit shifting
// don't change this only works with 8
#define field_width 8
#define field_height 10

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

// positive difference between two numbers
#define diff(a, b) ((a) > (b) ? (a) - (b) : (b) - (a))
// should receive good optimization if horizontal is a constant
// mirrors ball direction
#define mirror_direction(direction, horizontal)                                \
    (direction_max -                                                           \
     (horizontal ? direction                                                   \
                 : ((direction + (direction_max >> 1)) % direction_max)))

// set sprite and also place it in one stroke
#define move_set_sprite(nb, tile, x, y)                                        \
    set_sprite_tile(nb, tile);                                                 \
    move_sprite(nb, x, y);

// converts random number into block index
UINT8 random_block(UINT8 ran);

// generate a level with random blocks
void random_level(UINT16 seed);

// generate a level containing only one type of block
void one_block_level(UINT8 block);

// directly modifies background variable
// always draws current_level
void draw_blocks(void);

// lock ball to paddle
UINT8 lock_ball(void);

// only gets called for ball.y < 16 and ball.y can't be < 0
// returns >0 if ball collides with paddle
UINT8 collision_paddle(UINT8 x);

// returns >0 if ball collides with a block
UINT8 collision_block(UINT8 position);

void change_paddle_size(UINT8 size);

void move_paddle(UINT8 by);

// sound effect generator
void plonger(UINT8 note);
void fade_in(void);

// load prebuilt or random level
void load_level(UINT8 random, UINT16 level);

// initialize device for game
void great_burst_init(void);

// points and left balls
void draw_stats(void);

// main function of the game
void great_burst(void);

#endif
