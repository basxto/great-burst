CC=lcc
emulator=retroarch -L /usr/lib/libretro/gambatte_libretro.so
pngconvert=./dev/GameBoyPngConverter/linux-x64/GameBoyPngConverter

build: pix/oga_splash_data.c pix/oga_splash_movable_data.c
	$(CC) -Wa-l -Wl-m -Wl-j -c -o main.o main.c
	$(CC) -Wa-l -Wl-m -Wl-j -o main.gb main.o

run:
	$(emulator) ./main.gb

%_data.c: %.png
	$(pngconvert) $^

clean:
	rm *.gb *.o *.map *.lst *.sym pix/*_map.c pix/*_data.c

test: build run
