#ifndef POCKET_LISP_H
#define POCKET_LISP_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>

typedef union PKAtomData_ *PKAtom;

typedef PKAtom (PKFun0)(void *user_closure);
typedef PKAtom (PKFun1)(void *user_closure, PKAtom arg1);
typedef PKAtom (PKFun2)(void *user_closure, PKAtom arg1, PKAtom arg2);
typedef PKAtom (PKFun3)(void *user_closure, PKAtom arg1, PKAtom arg2, PKAtom arg3);

#define PK_PRINTF_FMT(fmt_, vargs_) __attribute__((format (printf, fmt_, vargs_)))

typedef struct PKStr_ {
    char *c;
    size_t length;
} PKStr;

typedef struct PKAtoms_ {
    PKAtom *e;
    size_t length;
} PKAtoms;

#define PK_ATOMS_EMPTY (PKAtoms){.e = NULL, .length = 0}

typedef enum PK_ENV_ {
    PK_ENV_VAR = 0,
    PK_ENV_FUN,
    PK_ENV_PROP,
} PK_ENV;

typedef enum PK_READ_ {
    PK_READ_EXPRESSION = 0,
    PK_READ_EXPRLIST,
} PK_READ;

typedef enum PK_ARITY_ {
    PK_ARITY_NORMAL = 0,
    PK_ARITY_OPTIONAL,
    PK_ARITY_VARIADIC,
} PK_ARITY;

typedef enum PK_DUMP_ {
    PK_DUMP_NONE = 0,
    PK_DUMP_STACK = (1 << 0),
    PK_DUMP_ENV = (1 << 1),
    PK_DUMP_ALL = PK_DUMP_STACK | PK_DUMP_ENV,
} PK_DUMP;

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
        goto label; \
    } \
} while(0)

#define pk_defer(atom_) pk_deferc(atom_, DEFER)

#define pk_alen(array_) (sizeof(array_) / sizeof(*array_))

bool pk_init(void);
void pk_deinit(void);

PKAtom pk_ref_inc(PKAtom atom);
PKAtom pk_ref_inc_many(PKAtoms atoms);
void pk_ref_dec(PKAtom atom);

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
PKAtom pk_list_packed(PKAtoms atoms);
PKAtom pk_list_flatten(PKAtoms lists);
PKAtom pk_array(PKAtoms atoms);
PKAtom pk_table(void);

PKAtom pk_cfun0(void *user_closure, PKFun0 fun);
PKAtom pk_cfun1(void *user_closure, PKFun1 fun, PK_ARITY arity);
PKAtom pk_cfun2(void *user_closure, PKFun2 fun, PK_ARITY arity);
PKAtom pk_cfun3(void *user_closure, PKFun3 fun, PK_ARITY arity);

PKAtom pk_add(PKAtom lhs, PKAtom rhs);
PKAtom pk_sub(PKAtom lhs, PKAtom rhs);
PKAtom pk_mul(PKAtom lhs, PKAtom rhs);
PKAtom pk_div(PKAtom lhs, PKAtom rhs);

bool pk_eq(PKAtom lhs, PKAtom rhs);

PKAtom pk_set(PK_ENV env, PKAtom symbol, PKAtom value);
PKAtom pk_get(PK_ENV env, PKAtom symbol);
PKAtom pk_let(PK_ENV env, PKAtom symbol, PKAtom value);
bool pk_is_bound(PK_ENV env, PKAtom symbol);
PKAtom pk_unbind(PK_ENV env, PKAtom symbol);

PKAtom pk_print(PKAtom atom);
PKAtom pk_format(PKAtom atom);
PKAtom pk_read(PKAtom string, PK_READ mode);
PKAtom pk_read_str(PKStr string, PK_READ mode);
PKAtom pk_eval(PKAtom expr);
PKAtom pk_compile(PKAtom expr);
PKAtom pk_evlist(PKAtom exprlist);

PKAtom pk_funcall0(PKAtom fn);
PKAtom pk_funcall1(PKAtom fn, PKAtom arg1);
PKAtom pk_funcall2(PKAtom fn, PKAtom arg1, PKAtom arg2);
PKAtom pk_funcall3(PKAtom fn, PKAtom arg1, PKAtom arg2, PKAtom arg3);

bool pk_is_nil(PKAtom atom);
bool pk_is_true(PKAtom atom);
bool pk_is_error(PKAtom atom);
bool pk_is_number(PKAtom atom);
bool pk_is_string(PKAtom atom);
bool pk_is_symbol(PKAtom atom);
bool pk_is_keyword(PKAtom atom);
bool pk_is_cons(PKAtom atom);

PKAtom pk_typecheck_nil(PKAtom atom);
PKAtom pk_typecheck_number(PKAtom atom);
PKAtom pk_typecheck_string(PKAtom atom);
PKAtom pk_typecheck_symbol(PKAtom atom);
PKAtom pk_typecheck_keyword(PKAtom atom);
PKAtom pk_typecheck_cons(PKAtom atom);
    
PKStr pk_to_string(PKAtom atom);
int pk_to_int(PKAtom atom);
float pk_to_float(PKAtom atom);

bool pk_string_qeq(PKAtom string, PKStr comp);
bool pk_symbol_qeq(PKAtom symbol, PKStr comp);
bool pk_keyword_qeq(PKAtom keyword, PKStr comp);

PKAtom pk_debug_dump(const char *tag, PK_DUMP opt);
PKAtom pk_hexdump(PKAtom lfun);

#endif // POCKET_LISP_H
