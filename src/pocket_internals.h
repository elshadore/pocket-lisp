#ifndef POCKET_INTERNALS_H
#define POCKET_INTERNALS_H

#include "pocket.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>

#define PK_WRITER_INIT_CAPACITY (256)
#define PK_LET_INIT_CAPACITY (256)
#define PK_INTERN_INIT_CAPACITY (64)
#define PK_STACK_INIT_CAPACITY (128)
#define PK_FRAMES_INIT_CAPACITY (64)
#define PK_SYMTABLE_INIT_CAPACITY (64)
#define PK_SET_INIT_CAPACITY (64)
#define PK_POOL_MAX (1024)
#define PK_ARENA_PAGE (1024 * 1024)
#define PK_ARENA_STACK_INIT_CAPACITY (8)

#define PK_COMMENT_CHAR ';'
#define PK_COMPTIME_CHAR '#'

typedef signed char pk_i8;
typedef signed short pk_i16;
typedef signed int pk_i32;

typedef unsigned char pk_u8;
typedef unsigned short pk_u16;
typedef unsigned int pk_u32;

typedef pk_u8 pk_bool;

#define PK_FALSE ((pk_u8)0u)
#define PK_TRUE ((pk_u8)1u)

typedef union PKAtom_ PKAtom;

typedef enum PKAtomTy_ {
    PKAtomTy_Free = 0,
    PKAtomTy_Nil,
    PKAtomTy_Number,
    PKAtomTy_Symbol,
    PKAtomTy_String,
    PKAtomTy_Cons,
    PKAtomTy_CFunc
} PKAtomTy;

typedef enum PKEnvTy_ {
    PKEnvTy_Var = 0,
    PKEnvTy_Fun
} PKEnvTy;

typedef struct PKAtomTag_ {
    PKAtomTy ty;
    pk_bool marked;
} PKAtomTag;

typedef struct PKAtomFree_ PKAtomFree;
struct PKAtomFree_ {
    PKAtomTag tag;
    PKAtomFree *next;
};

typedef enum PKNumberTy_ {
    PKNumberTy_Int = 0,
    PKNumberTy_Float
} PKNumberTy;

typedef struct PKAtomNumber_ {
    PKAtomTag tag;
    PKNumberTy ty;
    union {
        int i;
        float f;
    } as;
} PKAtomNumber;

typedef struct PKAtomString_ {
    PKAtomTag tag;
    char *c;
    size_t length;
    size_t hash;
} PKAtomString;

typedef struct PKAtomSymbol_ PKAtomSymbol;
struct PKAtomSymbol_ {
    PKAtomTag tag;
    PKAtomString *id;
    PKAtomSymbol *chain;
};

typedef struct PKAtomCons_ {
    PKAtomTag tag;
    PKAtom *car;
    PKAtom *cdr;
} PKAtomCons;

typedef struct PKFuncArity_ {
    PKArity mode;
    int args;
} PKFuncArity;

typedef struct PKFuncRecord_ {
    char *sym;
    PKFn fn;
    int args;
    PKArity mode;
    void *user_closure;
} PKFuncRecord;

typedef struct PKAtomCFunc_ {
    PKAtomTag tag;
    void *user_closure;
    PKFn fn;
    PKFuncArity arity;
} PKAtomCFunc;

union PKAtom_ {
    PKAtomTag tag;
    PKAtomFree free;
    PKAtomNumber number;
    PKAtomSymbol symbol;
    PKAtomString string;
    PKAtomCons cons;
    PKAtomCFunc cfunc;
};

typedef enum PKFuncMode_ {
    PKFuncMode_Func = 0,
    PKFuncMode_Macro
} PKFuncMode;

typedef enum PKFuncTy_ {
    PKFuncTy_CFunc = 0,
    PKFuncTy_Lambda
} PKFuncTy;

typedef struct PKCCall_ {
    void *user_closure;
    PKFn fn;
} PKCCall;

typedef struct PKLCall_ {
    PKAtom *args;
    PKAtom *body;
} PKLCall;

typedef struct PKCallConv_ {
    union {
        PKCCall c;
        PKLCall lisp;
    } as;
    PKFuncTy ty;
    PKFuncMode mode;
    size_t final_arity;
    size_t extra_nils;
    pk_bool insert_result;
    PKAtom *expression;
} PKCallConv;

typedef struct PKWriter_ {
    Pocket lisp;
    char *c;
    size_t count;
    size_t capacity;
} PKWriter;

typedef struct PKReader_ {
    Pocket lisp;
    char *c;
    size_t length;
    size_t curr;
} PKReader;

typedef struct PKLet_ {
    PKEnvTy ty;
    PKAtomSymbol *symbol;
    PKAtom *restore;
} PKLet;

typedef struct PKLets_ {
    PKLet *e;
    size_t count;
    size_t capacity;
} PKLets;

typedef struct PKStack_ {
    PKAtom **e;
    size_t count;
    size_t capacity;
} PKStack;

typedef enum PKEvalFrameTy_ {
    PKEvalFrameTy_None = 0,
    PKEvalFrameTy_Atom,
    PKEvalFrameTy_Tuple,
    PKEvalFrameTy_CFn
} PKEvalFrameTy;

typedef struct PKTuple_ {
    PKAtom *a;
    PKAtom *b;
} PKTuple;

typedef struct PKTCons_ {
    PKAtomCons *head;
    PKAtomCons *tail;
} PKTCons;

typedef struct PKCCons_ {
    PKAtomCons *cons;
    PKAtomCons *head;
    PKAtomCons *tail;
} PKCCons;

typedef struct PKHashTableSlot_ PKHashTableSlot;
struct PKHashTableSlot_ {
    PKAtom *key;
    PKAtom *value;
    PKHashTableSlot *chain;
};

typedef struct PKHashTable_ {
    PKHashTableSlot **e;
    size_t count;
    size_t capacity;
} PKHashTable;

typedef struct PKFrame_ {
    size_t stack_offset;
    size_t arity;
    size_t lets_offset;
} PKFrame;

typedef struct PKFrames_ {
    PKFrame *e;
    size_t count;
    size_t capacity;
} PKFrames;

typedef struct PKIntern_ {
    PKAtomSymbol **e;
    size_t count;
    size_t capacity;
} PKIntern;

typedef struct PKSymTableSlot_ PKSymTableSlot;
struct PKSymTableSlot_ {
    PKAtomSymbol *key;
    PKAtom *value;
    PKSymTableSlot *chain;
};

typedef struct PKSymTable_ {
    PKSymTableSlot **e;
    size_t count;
    size_t capacity;
} PKSymTable;

typedef struct PKPool_ PKPool;
struct PKPool_ {
    PKAtom e[PK_POOL_MAX];
    PKPool *next;
};

typedef struct PKAtoms_ {
    PKAtom **e;
    size_t length;
} PKAtoms;

typedef enum PKOrder_ {
    PKOrder_Normal = 0,
    PKOrder_Reversed
} PKOrder;

typedef struct PKStackSlice_ {
    PKOrder order;
    PKAtoms slice;
} PKStackSlice;

typedef struct PKArena_ {
    void *e;
    size_t head;
    size_t size;
} PKArena;

typedef struct PKArenaStack_ {
    PKArena *e;
    size_t count;
    size_t capacity;
} PKArenaStack;

typedef struct PKCache_ {
    PKAtom *nil;
    PKAtomSymbol *nil_sym;
    PKAtomSymbol *t;
    PKAtomSymbol *lambda;
    PKAtomSymbol *macro;
    PKAtomSymbol *quote;
    PKAtomSymbol *quasiquote;
    PKAtomSymbol *unquote;
    PKAtomSymbol *unquote_splice;
    PKAtomSymbol *string_substitute;
    PKAtomSymbol *progn;
    PKAtomSymbol *if_sym;
    PKAtomSymbol *while_sym;
    PKAtomSymbol *error;
    PKAtomSymbol *let_sym;
    PKAtomSymbol *flet_sym;
    PKAtomSymbol *let_star;
    PKAtomString *empty_string;
} PKCache;

struct PocketLispMachine_ {
    void *user_env;
    PKAllocFn alloc;
    PKPrintFn print;
    
    PKCache cache;
    
    PKStack stack;
    PKLets lets;
    PKFrames frames;
    PKFrame current_frame;
    
    PKSymTable vars;
    PKSymTable funs;
    PKIntern intern;
    
    PKPool *pool;
    PKAtomFree *free;   
};

#define pk_index_inv(i_, length_) ((length_) - (i_) - 1)

#define pk_talloc(type_, lisp_, output_) \
pk_malloc(lisp_, sizeof(type_), (void **)(output_))

#define pk_tallocn(type_, lisp_, count_, output_) \
pk_malloc(lisp_, sizeof(type_) * count_, output_)

#define pk_trealloc(type_, lisp_, ptr_, old_count_, new_count_, output_) \
pk_realloc(lisp_, ptr_, sizeof(type_) * old_count_, sizeof(type_) * new_count_, output_)

#define pk_error(lisp) pk_error_impl(lisp, __FILE__, __LINE__)

#define pk_stack_length_total(lisp_) \
(lisp_)->stack.count

#define pk_stack_length_frame(lisp_) \
pk_stack_length_total(lisp_) - (lisp_)->current_frame.stack_offset

#define pk_string_free(lisp_, c_, length_) \
pk_free(lisp_, c_, length_)

size_t pk_grow_capacity(size_t old_capacity, size_t init_capacity);
size_t pk_next_pow2(size_t value);

PKRes pk_print(Pocket lisp, char *c, size_t length);
PKRes pk_puts(Pocket lisp, char *c, size_t length);

PKRes pk_atom_alloc(Pocket lisp, PKAtom **output);
void pk_atom_free(Pocket lisp, PKAtom *atom);

PKRes pk_atom_int(Pocket lisp, int value, PKAtomNumber **output);
PKRes pk_atom_float(Pocket lisp, float value, PKAtomNumber **output);

PKRes pk_atom_string(Pocket lisp, const char *cstr, PKAtomString **output);
PKRes pk_atom_stringn(Pocket lisp, const char *string, size_t length, PKAtomString **output);
PKRes pk_atom_stringn_nomemcpy(Pocket lisp, char *string, size_t length, PKAtomString **output);
PKRes pk_atom_string_concat(Pocket lisp, PKAtoms strings, PKAtomString **output);

PKRes pk_atom_symbol_uninterned(Pocket lisp, const char *cstr, PKAtomSymbol **output);
PKRes pk_atom_symbol_interned(Pocket lisp, const char *string, PKAtomSymbol **output);
PKRes pk_atom_symboln_uninterned(Pocket lisp, const char *cstr, size_t length, PKAtomSymbol **output);
PKRes pk_atom_symboln_interned(Pocket lisp, const char *string, size_t length, PKAtomSymbol **output);

PKRes pk_atom_cons(Pocket lisp, PKAtom *car, PKAtom *cdr, PKAtomCons **output);
PKRes pk_atom_cons_car(Pocket lisp, PKAtom *car, PKAtomCons **output);
PKRes pk_atom_cfunc(Pocket lisp, void *user_closure, PKFn fn, PKFuncArity arity, PKAtomCFunc **output);
PKRes pk_atom_nil_new(Pocket lisp, PKAtom **output);

PKType pk_atom_typeof(PKAtomTy ty);
    
PKAtom *pk_atom_nil(Pocket lisp);
PKAtom *pk_atom_t(Pocket lisp);

PKRes pk_atom_cast_number(Pocket lisp, PKAtom *atom, PKAtomNumber **output);
PKRes pk_atom_cast_string(Pocket lisp, PKAtom *atom, PKAtomString **output);
PKRes pk_atom_cast_symbol(Pocket lisp, PKAtom *atom, PKAtomSymbol **output);
PKRes pk_atom_cast_cons(Pocket lisp, PKAtom *atom, PKAtomCons **output);
PKRes pk_atom_cast_cfunc(Pocket lisp, PKAtom *atom, PKAtomCFunc **output);

pk_bool pk_atom_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
pk_bool pk_atom_symbol_eq(Pocket lisp, PKAtomSymbol *lhs, PKAtomSymbol *rhs);
pk_bool pk_atom_string_eq(Pocket lisp, PKAtomString *lhs, PKAtomString *rhs);
pk_bool pk_atom_is_true(PKAtom *atom);
pk_bool pk_atom_is_nil(PKAtom *atom);
pk_bool pk_atom_is_symbol(PKAtom *atom);

PKRes pk_string_dupe(Pocket lisp, const char *c, size_t length, char **output);
pk_bool pk_string_eq(const char *a, size_t a_length, const char *b, size_t b_length);

PKRes pk_atom_cons_tail(Pocket lisp, PKAtomCons *cons, PKAtomCons **output);
PKRes pk_atom_list2(Pocket lisp, PKAtom *first, PKAtom *second, PKAtomCons **output);

PKRes pk_malloc(Pocket lisp, size_t size, void **output);
PKRes pk_realloc(Pocket lisp, void *ptr, size_t old_size, size_t new_size, void **output);
void pk_free(Pocket lisp, void *ptr, size_t size);

PKRes pk_push(Pocket lisp, PKAtom *atom);
PKRes pk_sp_index(Pocket lisp, int stack_pointer, size_t *index);
PKRes pk_stack_head(Pocket lisp, PKAtom **output);
PKRes pk_stack_pop(Pocket lisp, PKAtom **output);
PKRes pk_stack_get(Pocket lisp, int stack_pointer, PKAtom **output);
PKAtoms pk_stack_slice(Pocket lisp);
PKRes pk_stack_slice_by(Pocket lisp, int a, int b, PKStackSlice *output);
PKRes pk_stack_set(Pocket lisp, int stack_pointer, PKAtom *atom);
size_t pk_stack_total(Pocket lisp);

PKRes pk_error_impl(Pocket lisp, const char *file, int line);

PKRes pk_number_add(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output);
PKRes pk_number_sub(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output);
PKRes pk_number_mul(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output);
PKRes pk_number_div(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output);
PKRes pk_number_mod(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output);

PKRes pk_number_lt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, pk_bool *output);
PKRes pk_number_lte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, pk_bool *output);
PKRes pk_number_gt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, pk_bool *output);
PKRes pk_number_gte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, pk_bool *output);
PKRes pk_number_eq(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, pk_bool *output);

int pk_number_to_int(PKAtomNumber *num);
float pk_number_to_float(PKAtomNumber *num);

PKWriter pk_writer_init(Pocket lisp);
PKRes pk_writer_deinit(PKWriter *w);
PKRes pk_writer_char(PKWriter *w, char c);
PKRes pk_writer_string(PKWriter *w, const char *string);
PKRes pk_writer_stringn(PKWriter *w, const char *string, size_t length);
PKRes pk_writer_string_escaped(PKWriter *w, const char *string);
PKRes pk_writer_stringn_escaped(PKWriter *w, const char *string, size_t length);
PKRes pk_writer_newline(PKWriter *w);
PKRes pk_writer_int(PKWriter *w, int integer);
PKRes pk_writer_float(PKWriter *w, float floater);
PKRes pk_writer_atom(PKWriter *w, PKAtom *atom);
PKRes pk_writer_get(PKWriter *w, char **out_c, size_t *out_length);
PKRes pk_writer_reset(PKWriter *w);
PKRes pk_writer_print(PKWriter *w);
PKRes pk_writer_address(PKWriter *w, size_t address);

pk_u8 pk_char_to_digit(char c);
pk_bool pk_char_is_hex(char c);
pk_u8 pk_char_to_hex(char c);
char pk_char_from_hex(pk_u8 byte);
pk_bool pk_char_is_digit(char c);
char pk_char_from_digit(pk_u8 integer);
pk_bool pk_char_is_whitespace(char c);
pk_bool pk_char_is_alphabet(char c);
pk_bool pk_char_is_symbol(char c);

/*
Push a frame to the stack, the *arity* param refers to the number of variables to take
from the previous frame.
*/
PKRes pk_frame_push(Pocket lisp, size_t arity);
/* Pop the current frame, clearing all stack allocated variables. */
PKRes pk_frame_pop_clear(Pocket lisp);
/* Pop the current frame, returning all the current stack allocated variables. */
PKRes pk_frame_pop_return(Pocket lisp);
/* Clear the current frame of all stack allocated variables. */
void pk_frame_clear(Pocket lisp);
size_t pk_frame_length(Pocket lisp);
size_t pk_frame_savepoint(Pocket lisp);
PKAtoms pk_frame_slice(Pocket lisp, PKFrame *frame, size_t length);
PKRes pk_let_push(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom *value);
PKRes pk_let_pop(Pocket lisp, size_t n);

size_t pk_hash_djb2(const char *c, size_t length);
size_t pk_hash_pointer(void *ptr);

PKRes pk_symtable_grow(Pocket lisp, PKSymTable *st);
PKRes pk_symtable_put(Pocket lisp, PKSymTable *st, PKAtomSymbol *key, PKAtom *value, PKAtom **output);
PKRes pk_symtable_get(Pocket lisp, PKSymTable *st, PKAtomSymbol *key, PKAtom **output);
PKRes pk_symtable_rem(Pocket lisp, PKSymTable *st, PKAtomSymbol *key, PKAtom **output);
PKRes pk_symtable_alist(Pocket lisp, PKSymTable *st, PKAtom **output);
void pk_symtable_deinit(Pocket lisp, PKSymTable *st);

PKRes pk_hashtable_grow(Pocket lisp, PKHashTable *ht);
PKRes pk_hashtable_get(Pocket lisp, PKHashTable *ht, PKAtom *key, PKAtom **output);
PKRes pk_hashtable_put(Pocket lisp, PKHashTable *ht, PKAtom *key, PKAtom *value);
void pk_hashtable_deinit(Pocket lisp, PKHashTable *ht);

PKRes pk_env_set(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom *value, PKAtom **output);
PKRes pk_env_get(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom **output);
PKRes pk_env_unbind(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom **output);

PKRes pk_gc_collect(Pocket lisp);
PKRes pk_load_std(Pocket lisp);

PKRes pk_slurp(Pocket lisp, const char *file_path, char **out_buffer, size_t *out_length);

PKRes pk_call(Pocket lisp, PKAtom *atom, size_t arity);
PKRes pk_callconv(Pocket lisp, PKAtom *atom, size_t arity, PKCallConv *output);
PKRes pk_bind_lambda_list(Pocket lisp, PKAtom *symbols, PKAtoms values);

PKRes pk_slice_list(Pocket lisp, PKAtoms atoms, PKAtom **output);
PKRes pk_slice_list_rev(Pocket lisp, PKAtoms atoms, PKAtom **output);
PKRes pk_slice_list_tailed(Pocket lisp, PKAtoms atoms, PKAtom **output);

void pk_arena_deinit(Pocket lisp, PKArena *arena);
void pk_arena_deinit_all(Pocket lisp);
PKRes pk_arena_alloc(Pocket lisp, size_t size, void **output);
void *pk_arena_savepoint(Pocket lisp);
void pk_arena_restore(Pocket lisp, void *ptr);

PKRes pk_ret_top(Pocket lisp);
PKRes pk_ret_nil(Pocket lisp);
PKRes pk_ret_this(Pocket lisp, PKAtom *atom);
PKRes pk_ret_all(Pocket lisp);
PKRes pk_ret_none(Pocket lisp);

PKRes pk_read_atom_string(PKReader *r, PKAtomString **string);
PKRes pk_read_atom_number(PKReader *r, PKAtomNumber **number);
PKRes pk_read_atom_symbol(PKReader *r, PKAtomSymbol **symbol);
PKRes pk_read_atom_cons(PKReader *r, PKAtom **output);
PKRes pk_read_atom_simple_macro(PKReader *r, PKAtomSymbol *macro, PKAtomCons **output);
PKRes pk_read_atom_unquote_macro(PKReader *r, PKAtomCons **output);
PKRes pk_read_atom(PKReader *r, PKAtom **atom);

PKRes pk_read_string(Pocket lisp, char *c, size_t length, PK_READ mode, PKAtom **output);

#endif
