#!/bin/sh
mkdir -p out
cc -O0 -std=c99 -Wall -Wextra -Werror repl.c -o out/pocket
