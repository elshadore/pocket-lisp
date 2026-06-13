#ifndef POCKET_H
#define POCKET_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <setjmp.h>

typedef struct PocketLispMachine_ *Pocket;

typedef void (*PKFn)(void *user_closure, Pocket *lisp);
typedef void *(*PKAllocFn)(void *user_closure, void *ptr, size_t old_size, size_t new_size);

typedef struct PKString_ {
    char *c;
    size_t length;
} PKString;

typedef enum PKType_ {
    PKType_Unknown = 0,
    PKType_Nil,
    PKType_Number,
    PKType_Symbol,
    PKType_String,
    PKType_Cons,
    PKType_Object,
} PKType;

void pk_pop(Pocket lisp);
void pk_popn(Pocket lisp, int n);
void pk_dupe(Pocket lisp, int stack_pointer);
void pk_swap(Pocket lisp, int a, int b);
void pk_insert(Pocket lisp, int stack_pointer);

int pk_get_top(Pocket lisp);
void pk_set_top(Pocket lisp, int new_top);

int pk_sp_absolute(Pocket lisp, int stack_pointer);
int pk_sp_relative(Pocket lisp, int stack_pointer);

void pk_set(Pocket lisp);
void pk_get(Pocket lisp);
void pk_unbind(Pocket lisp);

void pk_fset(Pocket lisp);
void pk_fget(Pocket lisp);
void pk_funbind(Pocket lisp);

void pk_setf(Pocket lisp);
void pk_getf(Pocket lisp);

void pk_push_t(Pocket lisp);
void pk_push_nil(Pocket lisp);
void pk_push_cond(Pocket lisp, bool cond);
void pk_push_int(Pocket lisp, int integer);
void pk_push_float(Pocket lisp, float floater);
void pk_push_string(Pocket lisp, PKString string);
void pk_push_cstr(Pocket lisp, char *cstr);
void pk_push_nstr(Pocket lisp, char *string, size_t length);
void pk_push_symbol(Pocket lisp, PKString symbol);
void pk_push_csym(Pocket lisp, char *cstr);
void pk_push_nsym(Pocket lisp, char *symbol, size_t length);
void pk_push_cons(Pocket lisp, int car, int cdr);

void pk_set_car(Pocket lisp);
void pk_set_cdr(Pocket lisp);
void pk_car(Pocket lisp);
void pk_cdr(Pocket lisp);

void pk_read(Pocket lisp);
void pk_read_cstr(Pocket lisp, char *cstr);
void pk_read_nstr(Pocket lisp, char *string, size_t length);

void pk_eval(Pocket lisp);
void pk_apply(Pocket lisp);
void pk_funcall(Pocket lisp, int args);

void pk_add(Pocket lisp);
void pk_sub(Pocket lisp);
void pk_div(Pocket lisp);
void pk_mul(Pocket lisp);

void pk_gt(Pocket lisp);
void pk_gte(Pocket lisp);
void pk_lt(Pocket lisp);
void pk_lte(Pocket lisp);

int pk_to_int(Pocket lisp, int stack_pointer);
float pk_to_float(Pocket lisp, int stack_pointer);
PKString pk_to_string(Pocket lisp, int stack_pointer);

Pocket pk_init(void *user_closure, PKAllocFn alloc);
void pk_deinit(Pocket lisp);

#endif
