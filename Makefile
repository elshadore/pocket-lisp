all:
	mkdir -p out
	clang -ggdb -O0 -Wall -Wextra repl.c -o out/pocket
