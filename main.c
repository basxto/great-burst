#include <gb/gb.h>
#include <stdio.h>

#include "splashscreen.h"
#include "great_burst.h"
#include "menu.h"

void main() {
    NR52_REG = 0x80; // enable sound
    NR50_REG = 0x77; // full volume
    NR51_REG = 0xFF; // all channels
    splash();
    //menu(0);
    great_burst();

}
