#ifndef MENU_H
#define MENU_H

#include <gb/gb.h>

#include "great_burst.h"

#define font_start 0x2E
#define font_space 0x04
#define font_offset 0x10
#define font_to_uppercase (font_offset + 0x20)

void write_line(UINT8 x, UINT8 y, UINT8 length, char *str);

void write_text(UINT8 x, UINT8 y, UINT8 width, UINT8 height, UINT8 offset,
                char *str, UINT8 length);

void draw_menu(UINT8 mode);

void help(void);

void credits(void);

void menu(UINT8 mode);

#endif
