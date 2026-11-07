#ifndef POCKET_LISP_H
#define POCKET_LISP_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>

#ifdef PK_CFLAG_SHARED
#define PK_API __attribute__((visibility("default")))
#else
#define PK_API
#endif

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

typedef enum PK_RES_ {
    PK_YIELDED = 0,
    PK_OK = 1,
} PK_RES;

typedef enum PK_ENV_ {
    PK_ENV_VAR = 0,
    PK_ENV_FUN,
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

typedef enum PK_CALL_ {
    PK_CALL_FUN = 0,
    PK_CALL_MACRO,
} PK_CALL;

#define pk_str(text_) (PKStr) {.c = (text_), .length = ((sizeof(text_) / sizeof(char)) - 1)}

#define PK_STR_EMPTY (PKStr){.c = NULL, .length = 0}

#define pk_alen(array_) (sizeof(array_) / sizeof(*array_))

PK_API Pocket pk_init(void *user_env);
PK_API void pk_deinit(Pocket lisp);

PK_API void pk_pop(Pocket lisp);
PK_API void pk_pop2(Pocket lisp);
PK_API void pk_pop3(Pocket lisp);
PK_API void pk_popn(Pocket lisp, size_t n);
PK_API void pk_swap(Pocket lisp, int a, int b);
PK_API void pk_dig(Pocket lisp, int stack_pointer);
PK_API void pk_dig_to(Pocket lisp, int from, int to);
PK_API void pk_swap_pop2(Pocket lisp);
PK_API void pk_swap_pop3(Pocket lisp);

PK_API int pk_top(Pocket lisp);
PK_API void pk_set_top(Pocket lisp, int stack_pointer);
PK_API int pk_sp_absolute(Pocket lisp, int stack_pointer);
PK_API int pk_sp_relative(Pocket lisp, int stack_pointer);

PK_API void pk_push_nil(Pocket lisp);
PK_API void pk_push_true(Pocket lisp);
PK_API void pk_push_bool(Pocket lisp, bool boolean);
PK_API void pk_push_undefined(Pocket lisp);
PK_API void pk_push_int(Pocket lisp, int integer);
PK_API void pk_push_float(Pocket lisp, float floater);
PK_API void pk_push_string(Pocket lisp, PKStr string);
PK_API void pk_push_stringc(Pocket lisp, char *cstring);
PK_API PK_PRINTF_FMT(2, 3) void pk_push_stringf(Pocket lisp, const char *fmt, ...);
PK_API void pk_push_symbol(Pocket lisp, PKStr symbol);
PK_API void pk_push_symbolc(Pocket lisp, char *csymbol);
PK_API PK_PRINTF_FMT(2, 3) void pk_push_symbolf(Pocket lisp, const char *fmt, ...);
PK_API void pk_push_gensym(Pocket lisp);
PK_API void pk_push_keyword(Pocket lisp, PKStr keyword);
PK_API void pk_push_keywordc(Pocket lisp, char *ckeyword);
PK_API PK_PRINTF_FMT(2, 3) void pk_push_keywordf(Pocket lisp, const char *fmt, ...);
PK_API void pk_push_cons(Pocket lisp, int car, int cdr);
PK_API void pk_list(Pocket lisp, int head, int tail);
PK_API void pk_push_cfun(Pocket lisp, void *user_closure, PKFun fun, pk_u8 args, PK_ARITY arity, pk_u8 options);
PK_API void pk_push_cmacro(Pocket lisp, void *user_closure, PKFun fun, pk_u8 args, PK_ARITY arity, pk_u8 options);
PK_API void pk_push_array(Pocket lisp);
PK_API void pk_push_string_array(Pocket lisp);
PK_API void pk_push_byte_array(Pocket lisp);
PK_API void pk_push_table(Pocket lisp);
PK_API void pk_push_stream(Pocket lisp, FILE *stream, bool keep_open);

PK_API void pk_append(Pocket lisp, int obj, int element);
PK_API void pk_cat(Pocket lisp, int lhs, int rhs);
PK_API void pk_el(Pocket lisp, int obj, size_t index);
PK_API size_t pk_length(Pocket lisp, int obj);
PK_API void pk_put(Pocket lisp, int obj, int key, int value);
PK_API void pk_rem(Pocket lisp, int obj, int key);
PK_API void pk_find(Pocket lisp, int obj, int key);
PK_API bool pk_is_member(Pocket lisp, int obj, int key);

PK_API void pk_clone(Pocket lisp, int stack_pointer);
PK_API bool pk_is_circular(Pocket lisp, int stack_pointer);
PK_API void pk_reverse(Pocket lisp, int seq);
PK_API void pk_to_list(Pocket lisp, int seq);

PK_API void pk_car(Pocket lisp, int cons);
PK_API void pk_cdr(Pocket lisp, int cons);
PK_API void pk_assoc(Pocket lisp, int alist, int key);
PK_API void pk_car_strict(Pocket lisp, int cons);
PK_API void pk_cdr_strict(Pocket lisp, int cons);

PK_API void pk_set_car(Pocket lisp, int cons, int newcar);
PK_API void pk_set_cdr(Pocket lisp, int cons, int newcdr);

PK_API void pk_add(Pocket lisp, int lhs, int rhs);
PK_API void pk_sub(Pocket lisp, int lhs, int rhs);
PK_API void pk_mul(Pocket lisp, int lhs, int rhs);
PK_API void pk_div(Pocket lisp, int lhs, int rhs);
PK_API void pk_mod(Pocket lisp, int lhs, int rhs);

PK_API void pk_let(Pocket lisp, PK_ENV env, int symbol, int value);
PK_API void pk_set(Pocket lisp, PK_ENV env, int symbol, int value);
PK_API void pk_get(Pocket lisp, PK_ENV env, int symbol);
PK_API void pk_get_raw(Pocket lisp, PK_ENV env, int symbol);
PK_API bool pk_is_bound(Pocket lisp, PK_ENV env, int symbol);
PK_API void pk_unbind(Pocket lisp, PK_ENV env, int symbol);

PK_API void pk_list_funs(Pocket lisp);
PK_API void pk_list_vars(Pocket lisp);
PK_API void pk_list_types(Pocket lisp);
PK_API void pk_list_slots(Pocket lisp, int object);
PK_API void pk_list_methods(Pocket lisp, int type);
PK_API void pk_list_implementors(Pocket lisp, int method);
PK_API void pk_list_symbols(Pocket lisp);

PK_API void pk_print(Pocket lisp, int atom);
PK_API void pk_write(Pocket lisp, int atom);
PK_API void pk_format(Pocket lisp, int atom);
PK_API void pk_hexdump(Pocket lisp, int atom);
PK_API void pk_parse(Pocket lisp, int string, PK_MODE mode);

PK_API bool pk_macroexpand1(Pocket lisp, int atom);
PK_API bool pk_macroexpand(Pocket lisp, int atom);

PK_API void pk_eval(Pocket lisp, int expr, PK_MODE mode);
PK_API void pk_apply(Pocket lisp, PK_CALL call, int fun, int args);
PK_API void pk_funcall(Pocket lisp, PK_CALL call, size_t args);
PK_API void pk_fastcall(Pocket lisp, void *user_closure, PKFun fun, size_t args);

PK_API PK_RES pk_protect(Pocket lisp, void *user_closure, PKFun fun, size_t args);
PK_API PK_RES pk_protect_noreturn(Pocket lisp, void *user_closure, PKFun fun, size_t args);
PK_API PK_RES pk_catch(Pocket lisp, int symbol, void *user_closure, PKFun fun, size_t args);

PK_API void pk_propagate(Pocket lisp);

PK_API void pk_push_throw_type(Pocket lisp);
PK_API void pk_push_throw_value(Pocket lisp);

PK_API void pk_throw(Pocket lisp, int symbol, int value);
PK_API void pk_error(Pocket lisp, int message);
PK_API void pk_errors(Pocket lisp, PKStr message);
PK_API PK_PRINTF_FMT(2, 3) void pk_errorf(Pocket lisp, const char *fmt, ...);
PK_API void pk_error_oom(Pocket lisp);

PK_API bool pk_is_type(Pocket lisp, int atom, int type);
PK_API void pk_assert_type(Pocket lisp, int atom, int type);

PK_API void pk_define_type_c_owned(Pocket lisp, int symbol, size_t size);
PK_API void pk_define_type_c_user(Pocket lisp, int symbol);
PK_API void pk_define_type_lisp(Pocket lisp, int symbol, int slots);

PK_API void pk_new(Pocket lisp, int type);

PK_API void pk_typeof(Pocket lisp, int stack_pointer);
PK_API void pk_slot_set(Pocket lisp, int object, int slot, int value);
PK_API void pk_slot_get(Pocket lisp, int object, int slot);
PK_API void pk_method_set(Pocket lisp, int type, int symbol, int method);
PK_API void pk_method_get(Pocket lisp, int type, int symbol);
PK_API void pk_method_get_raw(Pocket lisp, int type, int symbol);

PK_API bool pk_lt(Pocket lisp, int lhs, int rhs);
PK_API bool pk_lte(Pocket lisp, int lhs, int rhs);
PK_API bool pk_gt(Pocket lisp, int lhs, int rhs);
PK_API bool pk_gte(Pocket lisp, int lhs, int rhs);
PK_API bool pk_eq(Pocket lisp, int lhs, int rhs);
PK_API bool pk_object_eq(Pocket lisp, int lhs, int rhs);
PK_API bool pk_struct_eq(Pocket lisp, int lhs, int rhs);

PK_API void *pk_cobject_data(Pocket lisp, int cobject);
PK_API size_t pk_cobject_size(Pocket lisp, int cobject);

PK_API int pk_to_int(Pocket lisp, int number);
PK_API float pk_to_float(Pocket lisp, int number);

PK_API void pk_map(Pocket lisp, int seq, int fun);
PK_API void pk_filter(Pocket lisp, int seq, int fun);
PK_API void pk_fap(Pocket lisp, int seq, int fun);
PK_API void pk_reduce(Pocket lisp, int seq, int fun, int init);

#ifdef PK_CFLAG_DEBUG
PK_API void pk_debug_leakcheck(Pocket lisp, const char *tag);
PK_API void pk_debug_stackdump(Pocket lisp, const char *tag);
PK_API void pk_debug_envdump(Pocket lisp, const char *tag);
#endif

#endif // POCKET_LISP_H
