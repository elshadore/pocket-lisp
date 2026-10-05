#ifndef POCKET_LISP_H
#define POCKET_LISP_H

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

#define PK_ARRAY_EMPTY (PKAtoms){.e = NULL, .length = 0}

typedef enum PK_ENV_ {
    PK_ENV_VAR = 0,
    PK_ENV_FUN,
    PK_ENV_PROP,
} PK_ENV;

typedef enum PK_READ_ {
    PK_READ_EXPRESSION = 0,
    PK_READ_EXPRLIST,
} PK_READ;

#define pk_str(text_) (PKStr) {.c = (text_), .length = (sizeof(text_) - 1)}
#define PK_STR_EMPTY (PKStr){.c = NULL, .length = 0}

#define pk_propagate(atom_) do { \
    PKAtom temp_ = (atom_); \
    if (pk_is_error(temp_)) { \
        return temp_; \
    } \
} while(0)

#define pk_deferc(atom_, label_) do { \
    PKAtom temp_ = (atom_); \
    if (pk_is_error(temp_)) { \
        goto label_; \
    } \
} while(0)

#define pk_defer(atom_) pk_deferc(atom_, DEFER)

bool pk_init(void);
void pk_deinit(void);

PKAtom pk_ref_inc(PKAtom atom);
PKAtom pk_ref_dec(PKAtom atom);

PKAtom pk_true(void);
PKAtom pk_nil(void);
PKAtom pk_int(int integer);
PKAtom pk_float(float floater);
PKAtom pk_string(PKStr string);
PKAtom pk_symbol(PKStr symbol);
PKAtom pk_keyword(PKStr keyword);
PKAtom pk_cons(PKAtom car, PKAtom cdr);
PKAtom pk_list(PKAtoms atoms);
PKAtom pk_list2(PKAtom first, PKAtom second);
PKAtom pk_array(PKAtoms atoms);
PKAtom pk_table(void);

PKAtom pk_add(PKAtom lhs, PKAtom rhs);
PKAtom pk_sub(PKAtom lhs, PKAtom rhs);
PKAtom pk_mul(PKAtom lhs, PKAtom rhs);
PKAtom pk_div(PKAtom lhs, PKAtom rhs);

PKAtom pk_set(PK_ENV env, PKAtom symbol, PKAtom value);
PKAtom pk_get(PK_ENV env, PKAtom symbol);
bool pk_is_bound(PK_ENV env, PKAtom symbol);
PKAtom pk_unbind(PK_ENV env, PKAtom symbol);

PKAtom pk_print(PKAtom atom);
PKAtom pk_format(PKAtom atom);
PKAtom pk_read(PKAtom string, PK_READ mode);
PKAtom pk_read_str(PKStr string, PK_READ mode);
PKAtom pk_eval(PKAtom expr);
PKAtom pk_compile(PKAtom expr);
PKAtom pk_evlist(PKAtom exprlist);

bool pk_is_nil(PKAtom atom);
bool pk_is_error(PKAtom atom);
bool pk_is_number(PKAtom atom);
bool pk_is_string(PKAtom atom);
bool pk_is_symbol(PKAtom atom);
bool pk_is_cons(PKAtom atom);

#endif // POCKET_LISP_H
