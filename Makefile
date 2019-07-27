CC=lcc
emulator=retroarch -L /usr/lib/libretro/gambatte_libretro.so
pngconvert=./dev/GameBoyPngConverter/linux-x64/GameBoyPngConverter

build: pix/oga_splash_data.c pix/oga_splash_movable_data.c pix/great_burst_bg_data.c pix/great_burst_fg_data.c main.gb

main.gb: main.o splashscreen.o menu.o great_burst.o sound.o
	$(CC) -Wa-l -Wl-m -Wl-j -o $@ $^

run: main.gb
	$(emulator) ./main.gb

%.o: %.c
	$(CC) -Wa-l -Wl-m -Wl-j -c -o $@ $^

%_data.c: %.png
	$(pngconvert) $^

clean:
	rm *.gb *.o *.map *.lst *.sym pix/*_map.c pix/*_data.c

test: build run

base64:
	base64 main.gb | xclip -selection clipboard
