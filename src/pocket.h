#ifndef POCKET_LISP_H
#define POCKET_LISP_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>

typedef int8_t pk_i8;
typedef int16_t pk_i16;
typedef int32_t pk_i32;
typedef int64_t pk_i64;

typedef uint8_t pk_u8;
typedef uint16_t pk_u16;
typedef uint32_t pk_u32;
typedef uint64_t pk_u64;

typedef struct PocketLispMachine_ *Pocket;

typedef void (*PKFun)(void *user_closure, Pocket lisp);


#define PK_PRINTF_FMT(fmt_, vargs_) __attribute__((format (printf, fmt_, vargs_)))

typedef struct PKStr_ {
    char *c;
    size_t length;
} PKStr;

typedef enum PK_ENV_ {
    PK_ENV_VAR = 0,
    PK_ENV_FUN,
    PK_ENV_PROP,
} PK_ENV;

typedef enum PK_MODE_ {
    PK_MODE_EXPRESSION = 0,
    PK_MODE_EXPRLIST,
} PK_MODE;

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

typedef enum PK_CALL_ {
    PK_CALL_FUN = 0,
    PK_CALL_MACRO,
} PK_CALL;

typedef enum PK_TYPE_ {
    PK_TYPE_UNKNOWN = 0,
    PK_TYPE_TRUE,
    PK_TYPE_THROW,
    PK_TYPE_ERROR,
    PK_TYPE_NUMBER,
    PK_TYPE_NIL,
    PK_TYPE_INTEGER,
    PK_TYPE_FLOAT,
    PK_TYPE_STRING,
    PK_TYPE_SYMBOL,
    PK_TYPE_KEYWORD,
    PK_TYPE_LIST,
    PK_TYPE_CONS,
    PK_TYPE_SEQ,
    PK_TYPE_FUNCTION,
    PK_TYPE_CFUN,
    PK_TYPE_LFUN,
    PK_TYPE_MACRO,
    PK_TYPE_CMACRO,
    PK_TYPE_LMACRO,
    PK_TYPE_OBJECT,
    PK_TYPE_LOBJECT,
    PK_TYPE_COBJECT,
} PK_TYPE;

#define pk_str(text_) (PKStr) {.c = (text_), .length = (sizeof(text_) - 1)}
#define PK_STR_EMPTY (PKStr){.c = NULL, .length = 0}

#define pk_alen(array_) (sizeof(array_) / sizeof(*array_))

Pocket pk_init(void *user_env);
void pk_deinit(Pocket lisp);

void pk_pop(Pocket lisp);
void pk_pop2(Pocket lisp);
void pk_pop3(Pocket lisp);
void pk_popn(Pocket lisp, size_t n);
void pk_swap(Pocket lisp, int a, int b);
void pk_dig(Pocket lisp, int stack_pointer);
void pk_dig_to(Pocket lisp, int from, int to);
void pk_swap_pop2(Pocket lisp);
void pk_swap_pop3(Pocket lisp);

int pk_top(Pocket lisp);
void pk_set_top(Pocket lisp, int new_top);
int pk_sp_absolute(Pocket lisp, int stack_pointer);
int pk_sp_relative(Pocket lisp, int stack_pointer);

void pk_push_nil(Pocket lisp);
void pk_push_true(Pocket lisp);
void pk_push_bool(Pocket lisp, bool boolean);
void pk_push_undefined(Pocket lisp);
void pk_push_int(Pocket lisp, int integer);
void pk_push_float(Pocket lisp, float floater);
void pk_push_string(Pocket lisp, PKStr string);
void pk_push_stringc(Pocket lisp, char *cstring);
void pk_push_symbol(Pocket lisp, PKStr symbol);
void pk_push_symbolc(Pocket lisp, char *csymbol);
void pk_push_gensym(Pocket lisp);
void pk_push_keyword(Pocket lisp, PKStr keyword);
void pk_push_keywordc(Pocket lisp, char *ckeyword);
void pk_push_cons(Pocket lisp, int car, int cdr);
void pk_push_list(Pocket lisp, int head, int tail);
void pk_push_array(Pocket lisp, size_t count, int fill);
void pk_push_table(Pocket lisp);
void pk_push_cfun(Pocket lisp, void *user_closure, PKFun fun, pk_u8 args, PK_ARITY arity, pk_u8 options);
void pk_push_cmacro(Pocket lisp, void *user_closure, PKFun fun, pk_u8 args, PK_ARITY arity, pk_u8 options);
void pk_push_object(Pocket lisp, int type);

void pk_car(Pocket lisp, int cons);
void pk_cdr(Pocket lisp, int cons);
void pk_car_strict(Pocket lisp, int cons);
void pk_cdr_strict(Pocket lisp, int cons);

void pk_set_car(Pocket lisp, int cons, int newcar);
void pk_set_cdr(Pocket lisp, int cons, int newcdr);

void pk_add(Pocket lisp, int lhs, int rhs);
void pk_sub(Pocket lisp, int lhs, int rhs);
void pk_mul(Pocket lisp, int lhs, int rhs);
void pk_div(Pocket lisp, int lhs, int rhs);
void pk_mod(Pocket lisp, int lhs, int rhs);

void pk_let(Pocket lisp, PK_ENV env, int symbol, int value);
void pk_set(Pocket lisp, PK_ENV env, int symbol, int value);
void pk_get(Pocket lisp, PK_ENV env, int symbol);
void pk_get_raw(Pocket lisp, PK_ENV env, int symbol);
bool pk_is_bound(Pocket lisp, PK_ENV env, int symbol);
void pk_unbind(Pocket lisp, PK_ENV env, int symbol);
// void pk_env_list(Pocket lisp, PK_ENV env);

void pk_print(Pocket lisp, int atom);
void pk_write(Pocket lisp, int atom);
void pk_format(Pocket lisp, int atom);
void pk_read(Pocket lisp, int string, PK_MODE mode);
void pk_eval(Pocket lisp, int expr, PK_MODE mode);
void pk_apply(Pocket lisp, PK_CALL call, int fun, int args);
void pk_fcall(Pocket lisp, PK_CALL call, size_t args);
void pk_pcall(Pocket lisp, PK_CALL call, size_t args);
void pk_compile(Pocket lisp, PK_CALL type, int value, PK_MODE mode);
void pk_macroexpand1(Pocket lisp, int atom);
void pk_macroexpand(Pocket lisp, int atom);

void pk_throw(Pocket lisp, int symbol, int value);
void pk_error(Pocket lisp);
// void pk_error(Pocket lisp, int message);
void pk_error_oom(Pocket lisp);

bool pk_is_type(Pocket lisp, int atom, PK_TYPE check);
void pk_assert_type(Pocket lisp, int atom, PK_TYPE check);

void pk_deftype_lisp(Pocket lisp, int symbol, int slots);
void pk_deftype_c_owned(Pocket lisp, int symbol, size_t size);
void pk_deftype_c_user(Pocket lisp, int symbol);
void pk_slot_set(Pocket lisp, int object, int slot, int value);
void pk_slot_get(Pocket lisp, int object, int slot);
void pk_method_set(Pocket lisp, int type, int symbol, int method);
void pk_method_get(Pocket lisp, int type, int symbol);
void pk_method_get_raw(Pocket lisp, int type, int symbol);

bool pk_lt(Pocket lisp, int lhs, int rhs);
bool pk_lte(Pocket lisp, int lhs, int rhs);
bool pk_gt(Pocket lisp, int lhs, int rhs);
bool pk_gte(Pocket lisp, int lhs, int rhs);
bool pk_eq(Pocket lisp, int lhs, int rhs);

void pk_slice(Pocket lisp, int seq, int head, int tail);
void pk_reverse(Pocket lisp, int seq);
void pk_flatten(Pocket lisp, int seq);

void *pk_cobject_data(Pocket lisp, int cobject);
size_t pk_cobject_size(Pocket lisp, int cobject);

PKStr pk_to_string(Pocket lisp, int atom);
int pk_to_int(Pocket lisp, int number);
float pk_to_float(Pocket lisp, int number);

void pk_hexdump(Pocket lisp, int lfun);
void pk_stackdump(Pocket lisp);

#endif // POCKET_LISP_H
