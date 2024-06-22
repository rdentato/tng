# tng makefile

## Table of Contents
- [tng makefile](#tng-makefile)
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
  - [Config debug level](#config-debug-level)
  - [Compilation flags](#compilation-flags)
  - [Directory for auxiliary libraries](#directory-for-auxiliary-libraries)
  - [The `all` target](#the-all-target)
  - [The `tng` target](#the-tng-target)
  - [The `tng2html` target](#the-tng2html-target)
  - [Cleanup](#cleanup)
  - [Distribution](#distribution)

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
(after:Config)
EXTLIBS=../extlibs
```

## The `all` target

```makefile
(before:targets)
all:tng tng2htm

```

## The `tng` target

```makefile
(after:targets)
tng: tng.o $(EXTLIBS)/val.o
	$(CC) $(CFLAGS) -o tng tng.o $(EXTLIBS)/val.o

tng.c: tng.md
	./tng -o tng.c tng.md

```
## The `tng2html` target

```makefile
(after:targets)
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
(after:Cleanup)
clean:
	rm -f *.o tng.exe *.obj tng.c tng2htm.c tng tng2htm boot/*.o boot/*.obj

cleanall:
	rm -f *.o tng.exe *.obj tng.c tng2htm.c tng tng2htm makefile
	rm -f boot/*.o boot/*.obj $(EXTLIBS)/val.o  $(EXTLIBS)/val.obj  ../distr/*
```

## Distribution
  If you want to use `tng` for your project, it might be an hassle to have to download
and compile `tng` separately. 
  You can create a single-file distribution in the `distr` directory using this target:

```makefile
(after:Targets)
DISTR=../distr
DISTR_FILES=dbg.h try.h vrg.h val.h val.c ../src/tng.c
distr:
	echo "#define VRGCLI" > $(DISTR)/tng.c
	echo "#define NDEBUG" >> $(DISTR)/tng.c
	grep "define exception_info" tng.c >> $(DISTR)/tng.c
	cd $(EXTLIBS); for f in $(DISTR_FILES); do echo""; echo "#line 1 \"$$f\""; cat $$f; done >> $(DISTR)/tng.c
	cd $(DISTR); $(CC) -O3 -Wall -o tng tng.c
```
