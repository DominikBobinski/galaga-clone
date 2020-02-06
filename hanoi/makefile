TARGET=sun

all: $(TARGET)

%.o:%.c 
	gcc -g -I.. -Wall -pedantic `sdl2-config --cflags` -c  $< -o $@

%: %.c ../primlib.o %.c
	gcc -g -I.. -Wall -pedantic `sdl2-config --cflags` ../primlib.o  $< -o $@ -lSDL2_gfx `sdl2-config --libs`

../primlib.o: ../primlib.c ../primlib.h

$(TARGET): $(TARGET).c ../primlib.h

clean:
	-rm $(TARGET)