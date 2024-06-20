# tng makefile

## Table of Contents

## Introduction
  To build 

```makefile
#  SPDX-FileCopyrightText: © 2024 Remo Dentato <rdentato@gmail.com>
#  SPDX-License-Identifier: MIT

 (:Config)

 (:Targets)

 (:Cleanup)

```
## Config debug level

  You can set the level of debugging information by setting the `DEBUG` variable:

```makefile
(before:Config)
  DEBUG=-DDEBUG=DEBUG_TEST
```
  The possible values are:

  - `DEBUG_TEST` : The highest level with lots of information
  - `DEBUG_INFO` : Just some additional information
  - `DEBUG_ERROR`: Only fatal errors
  
  To disable debugging entirely (e.g. to create a "production" executable), you can define `NDEBUG` instead.
The simplest way is to uncomment the line below:

```makefile
(after:Config)
  #DEBUG=-DNDEBUG
```
## Compilation flags

```makefile
(before:Config)
  CFLAGS=-O2 -Wall $(DEBUG) -I$(EXTLIBS)
```
Uncomment the following line for extra optiomization
```makefile
(after:Config)
  #CFLAGS=-O3 -Wall -DNDEBUG -I$(EXTLIBS)
```
## Directory for auxiliary libraries

All dependencies are in the `extlibs` directory:
```makefile
EXTLIBS=../extlibs
```

## The `all` target

```makefile
(before:targets)
all:tng tng2htm
```

## The `tng` target

```makefile
tng: tng.o $(EXTLIBS)/val.o
	$(CC) $(CFLAGS) -o tng tng.o $(EXTLIBS)/val.o

tng.c: tng.md
	./tng -o tng.c tng.md

```
## The `tng2html` target

```makefile
tng2htm: tng2htm.o $(EXTLIBS)/val.o
	$(CC) $(CFLAGS) -o tng2htm tng2htm.o $(EXTLIBS)/val.o

tng2htm.c: tng tng2htm.md
	./tng -o tng2htm.c tng2htm.md

```
## Cleanup
  There are two targets for cleanup:
  - `clean` which removes all the temporary files
  - `cleanall` which removes everything, including the copiled libraries

```makefile
clean:
	rm -rf *.o tng.bak tng.c tng2htm.c tng tng2htm boot/*.o 

cleanall:
	rm -rf *.o tng.bak tng.c tng2htm.c tng tng2htm boot/*.o $(EXTLIBS)/val.o makefile
```
