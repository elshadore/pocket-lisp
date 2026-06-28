#!/bin/sh

set -e

mkdir -p out

./tools/file2h.sh repl.pk > repl.h 

CC="clang"
CFLAGS="-std=c99 -Wall -Wextra"
SRC=$(find src -name '*.c')

$CC $CFLAGS repl.c $SRC -o ./out/repl
