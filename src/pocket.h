#ifndef POCKET_H
#define POCKET_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

typedef struct PocketLispMachine_ *Pocket;

typedef enum PKRes_ {
    PK_Yield = 0,
    PK_Ok = 1,
} PKRes;

typedef PKRes (*PKFn)(void *user_closure, Pocket lisp);
typedef void *(*PKAllocFn)(void *user_env, void *ptr, size_t old_size, size_t new_size);
typedef void (*PKPrintFn)(void *user_env, char *c, size_t length);

typedef struct PKString_ {
    char *c;
    size_t length;
} PKString;

#define PK_PRINTF(fmt_, args_) __attribute__ ((format (printf, fmt_, args_)))

#define pkstr(text_) (PKString){.c = text_, .length = ((sizeof(text_) / sizeof(char)) - 1)}
#define PK_STRING_EMPTY (PKString){0}
#define pk_string_new(c_, length_) (PKString){.c = c_, .length = length_}
#define pk_alen(array_) (sizeof(array_) / sizeof(*array_))

#define pk_tryc(expr_, ret_) do { \
    if ((expr_) == PK_Yield) { \
        return (ret_); \
    } \
} while(0)

#define pk_try(expr_) pk_tryc(expr_, PK_Yield)

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
    PKType_Object = 6,
} PKType;

typedef enum PKArity_ {
    PKArity_Normal = 0,
    PKArity_Optional = 1,
    PKArity_Variadic = 2,
} PKArity;

PKRes pk_pop(Pocket lisp);
PKRes pk_popn(Pocket lisp, int n);
PKRes pk_dupe(Pocket lisp, int stack_pointer);
PKRes pk_swap(Pocket lisp, int a, int b);
PKRes pk_insert(Pocket lisp, int from, int to);
PKRes pk_list(Pocket lisp, int head, int tail);

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

// void pk_getf(Pocket lisp, int object, int accessor);
// void pk_getf_setf(Pocket lisp, int object, int accessor, int value);

PKRes pk_push_t(Pocket lisp);
PKRes pk_push_nil(Pocket lisp);
PKRes pk_push_cond(Pocket lisp, bool cond);
PKRes pk_push_int(Pocket lisp, int integer);
PKRes pk_push_float(Pocket lisp, float floater);
PKRes pk_push_string(Pocket lisp, PKString string);
PKRes pk_push_cstr(Pocket lisp, char *cstr);
PKRes pk_push_nstr(Pocket lisp, char *string, size_t length);
PKRes PK_PRINTF(2, 3) pk_push_printf(Pocket lisp, const char *fmt, ...);
PKRes pk_push_symbol(Pocket lisp, PKString symbol);
PKRes pk_push_csym(Pocket lisp, char *cstr);
PKRes pk_push_nsym(Pocket lisp, char *symbol, size_t length);
PKRes pk_push_cons(Pocket lisp, int car, int cdr);
PKRes pk_push_cfunc(Pocket lisp, void *user_closure, PKFn fn, int args, PKArity mode);

PKRes pk_set_car(Pocket lisp, int cons, int new_car);
PKRes pk_set_cdr(Pocket lisp, int cons, int new_cdr);
PKRes pk_car(Pocket lisp, int cons);
PKRes pk_cdr(Pocket lisp, int cons);

PKRes pk_read(Pocket lisp, int stack_pointer);
PKRes pk_read_cstr(Pocket lisp, char *cstr);
PKRes pk_read_nstr(Pocket lisp, char *string, size_t length);
PKRes pk_read_string(Pocket lisp, PKString string);

PKRes pk_format(Pocket lisp, int stack_pointer);

PKRes pk_eval(Pocket lisp, int stack_pointer);
PKRes pk_evlist(Pocket lisp, int stack_pointer);
PKRes pk_apply(Pocket lisp, int function, int args_lists);
PKRes pk_funcall(Pocket lisp, int arity);
PKRes pk_fastcall(void *user_closure, Pocket lisp, PKFn fn, int arity);

PKRes pk_add(Pocket lisp, int lhs, int rhs);
PKRes pk_sub(Pocket lisp, int lhs, int rhs);
PKRes pk_div(Pocket lisp, int lhs, int rhs);
PKRes pk_mul(Pocket lisp, int lhs, int rhs);

PKRes pk_gt(Pocket lisp, int lhs, int rhs, bool *output);
PKRes pk_gte(Pocket lisp, int lhs, int rhs, bool *output);
PKRes pk_lt(Pocket lisp, int lhs, int rhs, bool *output);
PKRes pk_lte(Pocket lisp, int lhs, int rhs, bool *output);
PKRes pk_eq(Pocket lisp, int lhs, int rhs, bool *output);

PKRes pk_to_int(Pocket lisp, int stack_pointer, int *output);
PKRes pk_to_float(Pocket lisp, int stack_pointer, float *output);
PKRes pk_to_string(Pocket lisp, int stack_pointer, PKString *output);

Pocket pk_init(void *user_closure, PKAllocFn alloc, PKPrintFn print);
void pk_deinit(Pocket lisp);

PKRes pk_stack_dump(Pocket lisp, const char *tag);
PKRes pk_trace_dump(Pocket lisp, const char *tag);
PKRes pk_env_dump(Pocket lisp, const char *tag);

#endif
