# AGENTS.md
This is the pocket lisp repository. Pocket lisp is a common lisp inspired scripting language written in C89.

## Structure
- `repl.c` The default repl that uses the pocket lisp library.
- `src/` Contains all the core language source code.
- `src/pocket.h` The pocket lisp user api.
- `src/pocket_internals.h` All the internal apis and shared structs.
- `src/pocket_all.c` All the c source files in one .c file for easy linking and building.
- `emacs/pocket-lisp-mode.el` Contains the emacs mode for pocket lisp.
- `lib/` Contains thirdparty libraries used by pocket lisp.

## Repl Build
Run the `./build.sh` shell script to build the default repl.

## Style
Pocket lisp is written in *strict* C89 `-std=c89 -ggdb -Wall -Wextra -Werror -pedantic`. Pocket lisp uses braces for clear logic and four space indentation. Logic is written out in a clear and consistant manner. Below is an example of the way to write switch-cases and if statements.
``c
if (1) {
    printf("example");
}
int foo = 8;
switch (foo) {
    case 0: {
        printf("example");
        break;
    }
    case 8: {
        printf("another example");
        break;
    }
    default: {
        break;
    }
}
``
