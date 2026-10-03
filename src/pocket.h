#ifndef POCKET_H
#define POCKET_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

typedef union PKAtomData_ *PKAtom;

typedef PKAtom (PKFn)(void *user_closure, PKAtom atom);

#define PK_PRINTF_FMT(fmt_, vargs_) __attribute__((format (printf, fmt_, vargs_)))

typedef struct PKStr_ {
    char *c;
    size_t length;
} PKStr;

typedef struct PKAtoms_ {
    PKAtom *e;
    size_t length;
} PKAtoms;

#define pk_str(text_) (PKStr) {.c = (text_), .length = (sizeof(text_) - 1)}
#define PK_STR_EMPTY (PKStr){.c = NULL, .length = 0}

#define pk_propagate(atom_) do { \
    PKAtom temp_ = (atom_); \
    if (pk_is_error(temp_)) { \
        return temp_; \
    } \
} while(0)

bool pk_init(void);
void pk_deinit(void);

PKAtom pk_ref_inc(PKAtom atom);
PKAtom pk_ref_dec(PKAtom atom);

PKAtom pk_env(void *user_env);
PKAtom pk_true(void);
PKAtom pk_nil(void);
PKAtom pk_int(int integer);
PKAtom pk_float(float floater);
PKAtom pk_string(PKStr string);
PKAtom pk_symbol(PKStr symbol);
PKAtom pk_cons(PKAtom car, PKAtom cdr);
PKAtom pk_list(PKAtoms atoms);
PKAtom pk_array(PKAtoms atoms);

PKAtom pk_add(PKAtom lhs, PKAtom rhs);
PKAtom pk_sub(PKAtom lhs, PKAtom rhs);
PKAtom pk_mul(PKAtom lhs, PKAtom rhs);
PKAtom pk_div(PKAtom lhs, PKAtom rhs);

PKAtom pk_print(PKAtom atom);
PKAtom pk_read(PKAtom src);
PKAtom pk_format(PKAtom atom);

bool pk_is_nil(PKAtom atom);
bool pk_is_error(PKAtom atom);
bool pk_is_number(PKAtom atom);

PKAtom pk_for(void *user_closure, PKAtom seq, PKFn fn);
PKAtom pk_map(void *user_closure, PKAtom seq, PKFn fn);
PKAtom pk_fap(void *user_closure, PKAtom seq, PKFn fn);
PKAtom pk_filter(void *user_closure, PKAtom seq, PKFn fn);

#endif // POCKET_H
