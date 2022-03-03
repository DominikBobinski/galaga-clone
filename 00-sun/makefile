TARGET=sun

all: $(TARGET)

%.o:%.c 
	gcc -g -I.. -Wall -pedantic `sdl2-config --cflags` -c  $< -o $@

$(TARGET): $(TARGET).o ../primlib.o
	gcc -g -I.. -Wall -pedantic `sdl2-config --cflags` ../primlib.o  $< -o $@ -lSDL2_gfx `sdl2-config --libs` -lm

../primlib.o: ../primlib.c ../primlib.h

$(TARGET).o: $(TARGET).c ../primlib.h

clean:
	-rm $(TARGET) $(TARGET).o ../primlib.o