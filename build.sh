#!/bin/sh
mkdir -p out
./tools/pk2h.sh repl.pk repl.h REPL
./tools/pk2h.sh src/lisp/std.pk src/pocket_std.h PK_STD
cc -O0 -std=c99 -Wall -Wextra -Werror repl.c -o out/pocket
