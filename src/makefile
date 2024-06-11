
EXTLIBS=../extlibs

CFLAGS=-O2 -Wall -DDEBUG=DEBUG_TEST -I$(EXTLIBS)


all:tng

tng: tng.o $(EXTLIBS)/val.o
	$(CC) $(CFLAGES) -o tng tng.o $(EXTLIBS)/val.o

tng.c: tng_lit.c
	./tng.sh tng_lit.c

