#ifndef USER_LISP_H
#define USER_LISP_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

typedef union ULAtomData_ *ULAtom;

typedef ULAtom (ULFn)(void *user_closure, ULAtom atom);

#define UL_PRINTF_FMT(fmt_, vargs_) __attribute__((format (printf, fmt_, vargs_)))

typedef struct ULStr_ {
    char *c;
    size_t length;
} ULStr;

typedef struct ULAtoms_ {
    ULAtom *e;
    size_t length;
} ULAtoms;

typedef enum UL_ENV_ {
    UL_ENV_VAR = 0,
    UL_ENV_FUN,
    UL_ENV_PROP,
} UL_ENV;

#define ul_str(text_) (ULStr) {.c = (text_), .length = (sizeof(text_) - 1)}
#define UL_STR_EMPTY (ULStr){.c = NULL, .length = 0}

#define ul_propagate(atom_) do { \
    ULAtom temp_ = (atom_); \
    if (ul_is_error(temp_)) { \
        return temp_; \
    } \
} while(0)

bool ul_init(void);
void ul_deinit(void);

ULAtom ul_ref_inc(ULAtom atom);
ULAtom ul_ref_dec(ULAtom atom);

ULAtom ul_true(void);
ULAtom ul_nil(void);
ULAtom ul_int(int integer);
ULAtom ul_float(float floater);
ULAtom ul_string(ULStr string);
ULAtom ul_symbol(ULStr symbol);
ULAtom ul_cons(ULAtom car, ULAtom cdr);
ULAtom ul_list(ULAtoms atoms);
ULAtom ul_array(ULAtoms atoms);

ULAtom ul_add(ULAtom lhs, ULAtom rhs);
ULAtom ul_sub(ULAtom lhs, ULAtom rhs);
ULAtom ul_mul(ULAtom lhs, ULAtom rhs);
ULAtom ul_div(ULAtom lhs, ULAtom rhs);

ULAtom ul_set(UL_ENV env, ULAtom symbol, ULAtom value);
ULAtom ul_get(UL_ENV env, ULAtom symbol);
bool ul_is_bound(UL_ENV env, ULAtom symbol);
ULAtom ul_unbind(UL_ENV env, ULAtom symbol);

ULAtom ul_print(ULAtom atom);
ULAtom ul_read(ULAtom src);
ULAtom ul_format(ULAtom atom);

bool ul_is_nil(ULAtom atom);
bool ul_is_error(ULAtom atom);
bool ul_is_number(ULAtom atom);

ULAtom ul_for(void *user_closure, ULAtom seq, ULFn fn);
ULAtom ul_map(void *user_closure, ULAtom seq, ULFn fn);
ULAtom ul_fap(void *user_closure, ULAtom seq, ULFn fn);
ULAtom ul_filter(void *user_closure, ULAtom seq, ULFn fn);

#endif // USER_LISP_H
