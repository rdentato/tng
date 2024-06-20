#!/usr/bin/env bash
#  SPDX-FileCopyrightText: © 2024 Remo Dentato <rdentato@gmail.com>
#  SPDX-License-Identifier: MIT

CC=gcc
EXTLIBS=../extlibs

CFLAGS="-O2 -Wall -I$EXTLIBS"

$CC $CFLAGS -c -o $EXTLIBS/val.o $EXTLIBS/val.c
$CC $CFLAGS -c boot/tng_boot.c
$CC $CFLAGS -o tng tng_boot.o $EXTLIBS/val.o
rm -f tng_boot.o

./tng -n -o makefile makefile.md

make tng
