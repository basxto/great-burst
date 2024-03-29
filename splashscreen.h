#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <gb/gb.h>
#include <stdio.h>
#include <string.h>

#include "sound.h"

// offset in 8x8 tiles
#define splashscreen_a_xoffset 4
#define splashscreen_line_width 10

// make a bling sound
void bling(void);

// moves head in 'background' array
// starts on 5th (of 18) row, 2nd (of 20) column
// works from right to left and must operate this way
// 0x00 is a blank tile
void move_head(UINT8 offset);

// main function for splash screen animation
void splash(void);

#endif
