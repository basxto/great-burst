#include <gb/gb.h>
#include <stdio.h>

#include "splashscreen.c"

void main(){
    NR52_REG = 0x80; // enable sound
    NR50_REG = 0x77; // full volume
    NR51_REG = 0x11; // only first channel
    splash();

    while(1){
    }
}
