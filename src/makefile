
EXTLIBS=../extlibs

CFLAGS=-O2 -Wall -DDEBUG=DEBUG_TEST -I$(EXTLIBS)


all:tng tng2htm

tng: tng.o $(EXTLIBS)/val.o
	$(CC) $(CFLAGES) -o tng tng.o $(EXTLIBS)/val.o

tng2htm: tng2htm.o $(EXTLIBS)/val.o
	$(CC) $(CFLAGES) -o tng2htm tng2htm.o $(EXTLIBS)/val.o

tng.c: tng.md
	./tng -o tng.c tng.md

tng0: tng0.o $(EXTLIBS)/val.o
	$(CC) $(CFLAGES) -o tng tng0.o $(EXTLIBS)/val.o

tng0.c: tng_lit.c
	./tng.sh tng_lit.c
	cp tng.c tng0.c

tng2htm.c: tng tng2htm.md
	./tng -o tng2htm.c tng2htm.md
