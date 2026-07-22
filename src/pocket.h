#ifndef POCKET_H
#define POCKET_H

#include <stdlib.h>

typedef struct PocketLispMachine_ *Pocket;

typedef enum PKRes_ {
    PK_Yield = 0,
    PK_Ok = 1
} PKRes;

typedef PKRes (*PKFn)(void *user_closure, Pocket lisp);
typedef void *(*PKAllocFn)(void *user_env, void *ptr, size_t old_size, size_t new_size);
typedef void (*PKPrintFn)(void *user_env, char *c, size_t length);

#define pk_try(expr_) do { \
    if ((expr_) == PK_Yield) { \
        return PK_Yield; \
    } \
} while(0)

#define pk_deferc(expr_, jump_) do { \
    if ((expr_) == PK_Yield) { \
        goto jump_; \
    } \
} while(0)

#define pk_defer(expr_) pk_deferc(expr_, DEFER)

typedef enum PKType_ {
    PKType_Unknown = 0,
    PKType_Nil = 1,
    PKType_Number = 2,
    PKType_Symbol = 3,
    PKType_String = 4,
    PKType_Cons = 5,
    PKType_Object = 6
} PKType;

typedef enum PKVariant_ {
    PKVariant_Unknown = 0
} PKVariant;

typedef enum PKArity_ {
    PKArity_Normal = 0,
    PKArity_Optional = 1,
    PKArity_Variadic = 2
} PKArity;

typedef enum PK_READ_ {
    PK_READ_EXPRESSION = 0,
    PK_READ_LISTED = 1
} PK_READ;

PKRes pk_pop(Pocket lisp);
PKRes pk_popn(Pocket lisp, int n);
PKRes pk_dupe(Pocket lisp, int stack_pointer);
PKRes pk_swap(Pocket lisp, int a, int b);
PKRes pk_insert(Pocket lisp, int from, int to);

int pk_get_top(Pocket lisp);
PKRes pk_set_top(Pocket lisp, int new_top);

int pk_sp_absolute(Pocket lisp, int stack_pointer);
int pk_sp_relative(Pocket lisp, int stack_pointer);

PKRes pk_set(Pocket lisp, int symbol, int value);
PKRes pk_get(Pocket lisp, int symbol);
PKRes pk_unbind(Pocket lisp, int symbol);

PKRes pk_fset(Pocket lisp, int symbol, int value);
PKRes pk_fget(Pocket lisp, int symbol);
PKRes pk_funbind(Pocket lisp, int symbol);

PKRes pk_let(Pocket lisp, int symbol, int value);
PKRes pk_flet(Pocket lisp, int symbol, int value);

/* void pk_getf(Pocket lisp, int object, int accessor); */
/* void pk_setf(Pocket lisp, int object, int accessor, int value); */

PKRes pk_push_t(Pocket lisp);
PKRes pk_push_nil(Pocket lisp);
PKRes pk_push_cond(Pocket lisp, int cond);
PKRes pk_push_int(Pocket lisp, int integer);
PKRes pk_push_float(Pocket lisp, float floater);
PKRes pk_push_string(Pocket lisp, const char *cstr);
PKRes pk_push_stringn(Pocket lisp, const char *string, size_t length);
PKRes pk_push_symbol(Pocket lisp, const char *csym);
PKRes pk_push_symboln(Pocket lisp, const char *symbol, size_t length);
PKRes pk_push_cons(Pocket lisp, int car, int cdr);
PKRes pk_push_cfunc(Pocket lisp, void *user_closure, PKFn fn, int args, PKArity mode);
PKRes pk_list(Pocket lisp, int head, int tail);

PKRes pk_set_car(Pocket lisp, int cons, int new_car);
PKRes pk_set_cdr(Pocket lisp, int cons, int new_cdr);
PKRes pk_car(Pocket lisp, int cons);
PKRes pk_cdr(Pocket lisp, int cons);

PKRes pk_read(Pocket lisp, int stack_pointer, PK_READ mode);
PKRes pk_slurp(Pocket lisp, int file_path);
PKRes pk_format(Pocket lisp, int stack_pointer);
PKRes pk_clone(Pocket lisp, int stack_pointer);
PKRes pk_eval(Pocket lisp, int stack_pointer);
PKRes pk_evlist(Pocket lisp, int stack_pointer);
PKRes pk_apply(Pocket lisp, int function, int args);
PKRes pk_funcall(Pocket lisp, int arity);
PKRes pk_fastcall(void *user_closure, Pocket lisp, PKFn fn, int arity);

PKRes pk_add(Pocket lisp, int lhs, int rhs);
PKRes pk_sub(Pocket lisp, int lhs, int rhs);
PKRes pk_div(Pocket lisp, int lhs, int rhs);
PKRes pk_mul(Pocket lisp, int lhs, int rhs);

PKRes pk_gt(Pocket lisp, int lhs, int rhs, int *output);
PKRes pk_gte(Pocket lisp, int lhs, int rhs, int *output);
PKRes pk_lt(Pocket lisp, int lhs, int rhs, int *output);
PKRes pk_lte(Pocket lisp, int lhs, int rhs, int *output);
PKRes pk_eq(Pocket lisp, int lhs, int rhs, int *output);

PKRes pk_to_int(Pocket lisp, int stack_pointer, int *output);
PKRes pk_to_float(Pocket lisp, int stack_pointer, float *output);
PKRes pk_to_string(Pocket lisp, int stack_pointer, char **out_string, size_t *out_length);

PKRes pk_typeof(Pocket lisp, int stack_pointer, PKType *output);

PKRes pk_is_nil(Pocket lisp, int stack_pointer, int *output);
PKRes pk_is_number(Pocket lisp, int stack_pointer, int *output);
PKRes pk_is_symbol(Pocket lisp, int stack_pointer, int *output);
PKRes pk_is_string(Pocket lisp, int stack_pointer, int *output);
PKRes pk_is_cons(Pocket lisp, int stack_pointer, int *output);

Pocket pk_init(void *user_closure, PKAllocFn alloc, PKPrintFn print);
void pk_deinit(Pocket lisp);

PKRes pk_dump_stack(Pocket lisp, const char *tag);

#endif
