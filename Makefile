CC=lcc
emulator=retroarch -L /usr/lib/libretro/gambatte_libretro.so
pngconvert=./dev/GameBoyPngConverter/linux-x64/GameBoyPngConverter

build: pix/oga_splash_data.c pix/oga_splash_movable_data.c pix/great_burst_bg_data.c pix/great_burst_fg_data.c main.gb

#-Wl-m map output generated as outfile.map
#-Wl-j NoICE Debug output as outfile.cdb
# -Wl-yp0x143=0x80 gameboy mode
main.gb: main.o splashscreen.o menu.o great_burst.o sound.o
	$(CC) -Wl-m -Wl-j -Wl-yp0x143=0x80 -o $@ $^

run: main.gb
	$(emulator) ./main.gb

#-Wa-l create list output outfile.lst
%.o: %.c
	$(CC) -Wa-l -c -o $@ $^

%_data.c: %.png
	$(pngconvert) $^

clean:
	rm *.gb *.o *.map *.lst *.sym pix/*_map.c pix/*_data.c

test: build run

base64:
	base64 main.gb | xclip -selection clipboard
