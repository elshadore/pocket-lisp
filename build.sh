#!/bin/sh

set -e

mkdir -p out

CC="clang"
CFLAGS="-std=c89 -ggdb -Wall -Wextra -Werror -pedantic"
SRC=$(find src -name '*.c')

$CC $CFLAGS repl.c $SRC -o ./out/repl
