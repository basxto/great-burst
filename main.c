// original gameboy
#include <gb/gb.h>
// gameboy color
#include <gb/cgb.h>
#include <stdio.h>

#include "great_burst.h"
#include "menu.h"
#include "splashscreen.h"

void main() {
    NR52_REG = 0x80; // enable sound
    NR50_REG = 0x77; // full volume
    NR51_REG = 0xFF; // all channels

    cgb_compatibility();
    splash();
    menu(0);
}
