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
} PK_TYPE;

#define pk_str(text_) (PKStr) {.c = (text_), .length = (sizeof(text_) - 1)}
#define PK_STR_EMPTY (PKStr){.c = NULL, .length = 0}

#define pk_propagate(atom_) do { \
    PKAtom temp_ = (atom_); \
    if (pk_is_throwing(temp_)) { \
        return temp_; \
    } \
} while(0)

#define pk_deferc(atom_, label_) do { \
    PKAtom temp_ = (atom_); \
    if (pk_is_throwing(temp_)) { \
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
void pk_ref_dec_many(PKAtoms atoms);

PKAtom pk_true(void);
PKAtom pk_nil(void);
PKAtom pk_bool(bool boolean);
PKAtom pk_int(int integer);
PKAtom pk_float(float floater);
PKAtom pk_string(PKStr string);
PKAtom pk_stringc(char *cstring);
PKAtom pk_symbol(PKStr symbol);
PKAtom pk_symbolc(char *csymbol);
PKAtom pk_gensym(void);
PKAtom pk_keyword(PKStr keyword);
PKAtom pk_keywordc(char *ckeyword);
PKAtom pk_cons(PKAtom car, PKAtom cdr);
PKAtom pk_list(PKAtoms atoms);
PKAtom pk_list2(PKAtom first, PKAtom second);
PKAtom pk_list3(PKAtom first, PKAtom second, PKAtom third);
PKAtom pk_list_packed(PKAtoms atoms);
PKAtom pk_list_flatten(PKAtoms lists);
PKAtom pk_array(PKAtoms atoms);
PKAtom pk_table(void);

PKAtom pk_car(PKAtom cons);
PKAtom pk_cdr(PKAtom cons);
PKAtom pk_car_discard(PKAtom cons);
PKAtom pk_cdr_discard(PKAtom cons);

PKAtom pk_set_car(PKAtom cons, PKAtom new_car);
PKAtom pk_set_cdr(PKAtom cons, PKAtom new_cdr);

PKAtom pk_cfun0(void *user_closure, PK_CALL call, PKFun0 fun);
PKAtom pk_cfun1(void *user_closure, PK_CALL call, PKFun1 fun, PK_ARITY arity);
PKAtom pk_cfun2(void *user_closure, PK_CALL call, PKFun2 fun, PK_ARITY arity, pk_u8 optionals);
PKAtom pk_cfun3(void *user_closure, PK_CALL call, PKFun3 fun, PK_ARITY arity, pk_u8 optionals);

PKAtom pk_add(PKAtom lhs, PKAtom rhs);
PKAtom pk_sub(PKAtom lhs, PKAtom rhs);
PKAtom pk_mul(PKAtom lhs, PKAtom rhs);
PKAtom pk_div(PKAtom lhs, PKAtom rhs);
PKAtom pk_mod(PKAtom lhs, PKAtom rhs);
    
PKAtom pk_lt(PKAtom lhs, PKAtom rhs);
PKAtom pk_lte(PKAtom lhs, PKAtom rhs);
PKAtom pk_gt(PKAtom lhs, PKAtom rhs);
PKAtom pk_gte(PKAtom lhs, PKAtom rhs);
bool pk_eq(PKAtom lhs, PKAtom rhs);

PKAtom pk_set(PK_ENV env, PKAtom symbol, PKAtom value);
PKAtom pk_set_discard(PK_ENV env, PKAtom symbol, PKAtom value);
PKAtom pk_get(PK_ENV env, PKAtom symbol);
PKAtom pk_set_raw(PK_ENV env, PKAtom symbol, PKAtom value);
PKAtom pk_get_raw(PK_ENV env, PKAtom symbol);
PKAtom pk_list_env(PK_ENV env);
PKAtom pk_let(PK_ENV env, PKAtom symbol, PKAtom value);
bool pk_is_bound(PK_ENV env, PKAtom symbol);
PKAtom pk_unbind(PK_ENV env, PKAtom symbol);

PKAtom pk_print(PKAtom atom);
PKAtom pk_puts(PKAtom atom);
PKAtom pk_format(PKAtom atom);
PKAtom pk_read(PKAtom string, PK_MODE mode);
PKAtom pk_read_str(PKStr string, PK_MODE mode);
PKAtom pk_eval(PKAtom expr, PK_MODE mode);
PKAtom pk_apply(PK_CALL call, PKAtom fun, PKAtom args);
PKAtom pk_compile(PK_CALL type, PKAtom value, PK_MODE mode);
PKAtom pk_macroexpand1(PKAtom atom);
PKAtom pk_macroexpand(PKAtom atom);

PKAtom pk_funcall0(PK_CALL call, PKAtom fn);
PKAtom pk_funcall1(PK_CALL call, PKAtom fn, PKAtom arg1);
PKAtom pk_funcall2(PK_CALL call, PKAtom fn, PKAtom arg1, PKAtom arg2);
PKAtom pk_funcall3(PK_CALL call, PKAtom fn, PKAtom arg1, PKAtom arg2, PKAtom arg3);

PKAtom pk_reverse(PKAtom atom);

PKAtom pk_throw(PKAtom type, PKAtom contents);
PKAtom pk_throw_type(PKAtom throw);
PKAtom pk_throw_contents(PKAtom throw);
PKAtom pk_error(PKAtom message);
PKAtom pk_errors(PKStr message);
PKAtom pk_error_oom(void);
PKAtom pk_error_type(PK_TYPE expected, PKAtom found);
    
bool pk_is_nil(PKAtom atom);
bool pk_is_undefined(PKAtom atom);
bool pk_is_true(PKAtom atom);
bool pk_is_throwing(PKAtom atom);
bool pk_is_error(PKAtom atom);
bool pk_is_int(PKAtom atom);
bool pk_is_float(PKAtom atom);
bool pk_is_number(PKAtom atom);
bool pk_is_string(PKAtom atom);
bool pk_is_symbol(PKAtom atom);
bool pk_is_keyword(PKAtom atom);
bool pk_is_cons(PKAtom atom);
bool pk_is_function(PKAtom atom);
bool pk_is_macro(PKAtom atom);
bool pk_is_type(PKAtom atom, PK_TYPE type);

PKAtom pk_typecheck(PKAtom atom, PK_TYPE check);
PKAtom pk_typecheck_nil(PKAtom atom);
PKAtom pk_typecheck_number(PKAtom atom);
PKAtom pk_typecheck_string(PKAtom atom);
PKAtom pk_typecheck_symbol(PKAtom atom);
PKAtom pk_typecheck_keyword(PKAtom atom);
PKAtom pk_typecheck_cons(PKAtom atom);
    
PKStr pk_to_string(PKAtom atom);
PK_TYPE pk_type_of(PKAtom atom);
int pk_to_int(PKAtom atom);
float pk_to_float(PKAtom atom);

bool pk_string_qeq(PKAtom string, PKStr comp);
bool pk_symbol_qeq(PKAtom symbol, PKStr comp);
bool pk_keyword_qeq(PKAtom keyword, PKStr comp);

PKAtom pk_hexdump(PKAtom lfun);

#endif // POCKET_LISP_H
