all: testlib

testlib: testlib.o primlib.o
	gcc -g $^ -o $@  -lSDL2_gfx `sdl2-config --libs`

.c.o: 
	gcc -g -Wall -pedantic `sdl2-config --cflags` -c  $<

primlib.o: primlib.c primlib.h

testlib.o: testlib.c primlib.h

clean:
	-rm primlib.o testlib.o testlib