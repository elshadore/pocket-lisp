#!/bin/sh

set -e

mkdir -p out

CC="clang"
CFLAGS="-std=c89 -ggdb -Wall -Wextra -Werror -pedantic"

$CC $CFLAGS repl.c -o ./out/repl
