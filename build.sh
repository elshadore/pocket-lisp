#!/bin/sh

set -e

mkdir -p out

CC="clang"
CFLAGS="-std=c89 -Wextra -Werror -pedantic"
DEBUG="-ggdb -O0"
RELEASE="-O3"


$CC $CFLAGS $RELEASE repl.c -o ./out/repl
