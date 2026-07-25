#ifndef POCKET_INTERNALS_H
#define POCKET_INTERNALS_H

#include "pocket.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>

#define PK_WRITER_INIT_CAPACITY (256)
#define PK_LET_INIT_CAPACITY (256)
#define PK_INTERN_INIT_CAPACITY (64)
#define PK_STACK_INIT_CAPACITY (256)
#define PK_COMPILER_INIT_CAPACITY (4)
#define PK_FRAMES_INIT_CAPACITY (64)
#define PK_SYMTABLE_INIT_CAPACITY (64)
#define PK_SET_INIT_CAPACITY (64)
#define PK_POOL_MAX (1024)
#define PK_ARENA_PAGE (1024 * 1024)
#define PK_ARENA_STACK_INIT_CAPACITY (8)

#define PK_COMMENT_CHAR '#'
#define PK_COMPTIME_CHAR '&'

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
    PKAtomTy_CFunc,
    PKAtomTy_LFunc
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
    PK_ARITY mode;
    int args;
} PKFuncArity;

typedef struct PKFuncRecord_ {
    char *sym;
    PKFn fn;
    int args;
    PK_ARITY mode;
    void *user_closure;
} PKFuncRecord;

typedef struct PKAtomCFunc_ {
    PKAtomTag tag;
    void *user_closure;
    PKFn fn;
    PKFuncArity arity;
} PKAtomCFunc;

typedef struct PKAtomLFunc_ {
    PKAtomTag tag;
    struct {
        pk_u8 *e;
        size_t length;
    } bc;
    struct {
        PKAtom **e;
        size_t length;
    } atoms;
    PKFuncArity arity;
} PKAtomLFunc;

union PKAtom_ {
    PKAtomTag tag;
    PKAtomFree free;
    PKAtomNumber number;
    PKAtomSymbol symbol;
    PKAtomString string;
    PKAtomCons cons;
    PKAtomCFunc cfunc;
    PKAtomLFunc lfunc;
};

#define PK_CALLTY_QUICK (0)
#define PK_CALLTY_CFUNC (1)
#define PK_CALLTY_LFUNC (2)

typedef struct PKCallConv_ {
    union {
        struct {
            void *user_closure;
            PKFn fn;
        } quick;
        PKAtomCFunc *c;
        PKAtomLFunc *lisp;
    } as;
    pk_u8 ty;
    pk_bool insert_result;
    size_t final_arity;
    size_t extra_nils;
} PKCallConv;

typedef struct PKWriter_ {
    Pocket lisp;
    char *c;
    size_t count;
    size_t capacity;
} PKWriter;

typedef struct PKReader_ {
    Pocket lisp;
    char *string;
    size_t length;
    size_t curr;
    size_t line;
    size_t bol;
    char c;
} PKReader;

typedef struct PKVM_ {
    Pocket lisp;
    PKAtomLFunc *lfunc;
    size_t curr;
    pk_u8 op;
} PKVM;

typedef struct PKBytes_ {
    pk_u8 *e;
    size_t count;
    size_t capacity;
} PKBytes;

typedef struct PKAtoms_ {
    PKAtom **e;
    size_t count;
    size_t capacity;
} PKAtoms;

#define PK_OP_ILLEGAL (0)
#define PK_OP_NOP (1)
#define PK_OP_RET (2)
#define PK_OP_LOAD (3)
#define PK_OP_LOAD_NIL (4)
#define PK_OP_CALL (5)
#define PK_OP_BLOCK_BEGIN (6)
#define PK_OP_BLOCK_END (7)
#define PK_OP_JMP_IF_NIL (8)
#define PK_OP_JMP (9)
#define PK_OP_JMP_BACK (10)
#define PK_OP_LET_VAR (11)
#define PK_OP_LET_FUN (12)
#define PK_OP_LOOKUP_VAR (13)
#define PK_OP_LOOKUP_FUN (14)

typedef enum PK_OPCODE_TY_ {
    PK_OPCODE_TY_NORMAL = 0,
    PK_OPCODE_TY_LOAD,
    PK_OPCODE_TY_LIT
} PK_OPCODE_TY;

typedef struct PKCompiler_ {
    Pocket lisp;
    PKBytes bc;
    PKAtoms atoms;
    pk_u8 addr;
} PKCompiler;

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

typedef struct PKAtomSlice_ {
    PKAtom **e;
    size_t length;
} PKAtomSlice;

typedef enum PKOrder_ {
    PKOrder_Normal = 0,
    PKOrder_Reversed
} PKOrder;

typedef struct PKStackSlice_ {
    PKOrder order;
    PKAtomSlice slice;
} PKStackSlice;

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
    PKAtomSymbol *let_star;
    PKAtomSymbol *flet_sym;
    PKAtomSymbol *flet_star;
    PKAtomString *empty_string;
} PKCache;

typedef struct PKThrow_ {
    PKAtomSymbol *throw_sym;
    pk_bool throwing;
} PKThrow;

struct PocketLispMachine_ {
    void *user_env;
    PKAllocFn alloc;
    PKPrintFn print;
    
    PKCache cache;
   
    PKAtoms stack;
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
((lisp_)->stack.count)

#define pk_stack_length_frame(lisp_) \
(pk_stack_length_total(lisp_) - (lisp_)->current_frame.stack_offset)

#define pk_string_free(lisp_, c_, length_) \
pk_free(lisp_, c_, length_)

size_t pk_grow_capacity(size_t old_capacity, size_t init_capacity);
size_t pk_next_pow2(size_t value);

void pk_puts(Pocket lisp, char *c, size_t length);
void pk_print(Pocket lisp, char *c, size_t length);
void pk_newline(Pocket lisp);
PK_RES pk_print_atom(Pocket lisp, PKAtom *atom);
PK_RES pk_puts_atom(Pocket lisp, PKAtom *atom);

PK_RES pk_atom_alloc(Pocket lisp, PKAtom **output);
void pk_atom_free(Pocket lisp, PKAtom *atom);

PK_RES pk_atom_int(Pocket lisp, int value, PKAtomNumber **output);
PK_RES pk_atom_float(Pocket lisp, float value, PKAtomNumber **output);

PK_RES pk_atom_string(Pocket lisp, const char *cstr, PKAtomString **output);
PK_RES pk_atom_stringn(Pocket lisp, const char *string, size_t length, PKAtomString **output);
PK_RES pk_atom_stringn_nomemcpy(Pocket lisp, char *string, size_t length, PKAtomString **output);
PK_RES pk_atom_string_concat(Pocket lisp, PKAtomSlice strings, PKAtomString **output);
PK_RES pk_atom_string_slurp(Pocket lisp, PKAtomString *file_path, PKAtomString **output);

PK_RES pk_atom_symbol_uninterned(Pocket lisp, const char *cstr, PKAtomSymbol **output);
PK_RES pk_atom_symbol_interned(Pocket lisp, const char *string, PKAtomSymbol **output);
PK_RES pk_atom_symboln_uninterned(Pocket lisp, const char *cstr, size_t length, PKAtomSymbol **output);
PK_RES pk_atom_symboln_interned(Pocket lisp, const char *string, size_t length, PKAtomSymbol **output);

PK_RES pk_atom_cons(Pocket lisp, PKAtom *car, PKAtom *cdr, PKAtomCons **output);
PK_RES pk_atom_cons_car(Pocket lisp, PKAtom *car, PKAtomCons **output);
PK_RES pk_atom_cfunc(Pocket lisp, void *user_closure, PKFn fn, PKFuncArity arity, PKAtomCFunc **output);
PK_RES pk_atom_nil_new(Pocket lisp, PKAtom **output);

PK_TYPE pk_atom_typeof(PKAtomTy ty);
    
PKAtom *pk_atom_nil(Pocket lisp);
PKAtom *pk_atom_t(Pocket lisp);

PK_RES pk_atom_cast_number(Pocket lisp, PKAtom *atom, PKAtomNumber **output);
PK_RES pk_atom_cast_string(Pocket lisp, PKAtom *atom, PKAtomString **output);
PK_RES pk_atom_cast_symbol(Pocket lisp, PKAtom *atom, PKAtomSymbol **output);
PK_RES pk_atom_cast_cons(Pocket lisp, PKAtom *atom, PKAtomCons **output);
PK_RES pk_atom_cast_cfunc(Pocket lisp, PKAtom *atom, PKAtomCFunc **output);
PK_RES pk_atom_cast_lfunc(Pocket lisp, PKAtom *atom, PKAtomLFunc **output);
PK_RES pk_atom_assert_nil(Pocket lisp, PKAtom *atom);

pk_bool pk_atom_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
pk_bool pk_atom_symbol_eq(Pocket lisp, PKAtomSymbol *lhs, PKAtomSymbol *rhs);
pk_bool pk_atom_string_eq(Pocket lisp, PKAtomString *lhs, PKAtomString *rhs);

pk_bool pk_atom_is_true(PKAtom *atom);
pk_bool pk_atom_is_nil(PKAtom *atom);
pk_bool pk_atom_is_symbol(PKAtom *atom);

PK_RES pk_string_dupe(Pocket lisp, const char *c, size_t length, char **output);
pk_bool pk_string_eq(const char *a, size_t a_length, const char *b, size_t b_length);
void pk_string_reverse(char *c, size_t length);

PK_RES pk_atom_cons_tail(Pocket lisp, PKAtomCons *cons, PKAtomCons **output);
PK_RES pk_atom_list2(Pocket lisp, PKAtom *first, PKAtom *second, PKAtomCons **output);

PK_RES pk_malloc(Pocket lisp, size_t size, void **output);
PK_RES pk_realloc(Pocket lisp, void *ptr, size_t old_size, size_t new_size, void **output);
void pk_free(Pocket lisp, void *ptr, size_t size);

PK_RES pk_push(Pocket lisp, PKAtom *atom);
PK_RES pk_sp_index(Pocket lisp, int stack_pointer, size_t *index);
PK_RES pk_stack_head(Pocket lisp, PKAtom **output);
PKAtom *pk_stack_result(Pocket lisp);
PK_RES pk_stack_pop(Pocket lisp, PKAtom **output);
PK_RES pk_stack_get(Pocket lisp, int stack_pointer, PKAtom **output);
PKAtomSlice pk_stack_slice(Pocket lisp);
PK_RES pk_stack_slice_down(Pocket lisp, size_t depth, PKAtomSlice *output);
PK_RES pk_stack_slice_by(Pocket lisp, int a, int b, PKStackSlice *output);
PK_RES pk_stack_set(Pocket lisp, int stack_pointer, PKAtom *atom);

PK_RES pk_error_impl(Pocket lisp, const char *file, int line);

PK_RES pk_number_add(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output);
PK_RES pk_number_sub(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output);
PK_RES pk_number_mul(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output);
PK_RES pk_number_div(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output);
PK_RES pk_number_mod(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, PKAtomNumber **output);

PK_RES pk_number_lt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, pk_bool *output);
PK_RES pk_number_lte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, pk_bool *output);
PK_RES pk_number_gt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, pk_bool *output);
PK_RES pk_number_gte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, pk_bool *output);
PK_RES pk_number_eq(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, pk_bool *output);

int pk_number_to_int(PKAtomNumber *num);
float pk_number_to_float(PKAtomNumber *num);

PKWriter pk_writer_init(Pocket lisp);
PK_RES pk_writer_deinit(PKWriter *w);
PK_RES pk_writer_char(PKWriter *w, char c);
PK_RES pk_writer_string(PKWriter *w, const char *string);
PK_RES pk_writer_stringn(PKWriter *w, const char *string, size_t length);
PK_RES pk_writer_string_escaped(PKWriter *w, const char *string);
PK_RES pk_writer_stringn_escaped(PKWriter *w, const char *string, size_t length);
PK_RES pk_writer_newline(PKWriter *w);
PK_RES pk_writer_int(PKWriter *w, int integer);
PK_RES pk_writer_float(PKWriter *w, float floater);
PK_RES pk_writer_atom(PKWriter *w, PKAtom *atom);
PK_RES pk_writer_get(PKWriter *w, char **out_c, size_t *out_length);
PK_RES pk_writer_reset(PKWriter *w);
void pk_writer_print(PKWriter *w);
void pk_writer_puts(PKWriter *w);
PK_RES pk_writer_address(PKWriter *w, size_t address);

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
PK_RES pk_frame_push(Pocket lisp, size_t arity);
/* Pop the current frame, clearing all stack allocated variables. */
PK_RES pk_frame_pop(Pocket lisp);
/* Clear the current frame of all stack allocated variables. */
void pk_frame_clear(Pocket lisp);
/* TODO: detail the mechanics of this function */
void pk_frame_force_unwind(Pocket lisp, size_t i);
PKAtomSlice pk_frame_slice(Pocket lisp, PKFrame *frame, size_t length);
PK_RES pk_let_push(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom *value);
PK_RES pk_let_pop(Pocket lisp, size_t n);

size_t pk_hash_djb2(const char *c, size_t length);
size_t pk_hash_pointer(void *ptr);

PKSymTable pk_symtable_init(void);
PK_RES pk_symtable_grow(Pocket lisp, PKSymTable *st);
PK_RES pk_symtable_put(Pocket lisp, PKSymTable *st, PKAtomSymbol *key, PKAtom *value, PKAtom **output);
pk_u8 pk_symtable_get(Pocket lisp, PKSymTable *st, PKAtomSymbol *key, PKAtom **output);
PK_RES pk_symtable_rem(Pocket lisp, PKSymTable *st, PKAtomSymbol *key, PKAtom **output);
PK_RES pk_symtable_alist(Pocket lisp, PKSymTable *st, PKAtom **output);
void pk_symtable_deinit(Pocket lisp, PKSymTable *st);

PKHashTable pk_hashtable_init(void);
PK_RES pk_hashtable_grow(Pocket lisp, PKHashTable *ht);
pk_u8 pk_hashtable_get(Pocket lisp, PKHashTable *ht, PKAtom *key, PKAtom **output);
PK_RES pk_hashtable_put(Pocket lisp, PKHashTable *ht, PKAtom *key, PKAtom *value);
void pk_hashtable_deinit(Pocket lisp, PKHashTable *ht);

PK_RES pk_env_set(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom *value, PKAtom **output);
PK_RES pk_env_get(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom **output);
PK_RES pk_env_unbind(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom **output);

PK_RES pk_gc_collect(Pocket lisp);
PK_RES pk_load_std(Pocket lisp);

PK_RES pk_util_slurp(Pocket lisp, const char *file_path, char **out_buffer, size_t *out_length);
PK_RES pk_util_slurpn(Pocket lisp, const char *file_path, size_t length, char **out_c, size_t *out_length);

PK_RES pk_atom_eval(Pocket lisp, PKAtom *atom);
PK_RES pk_atom_evlist(Pocket lisp, PKAtom *atom);

PK_RES pk_call(Pocket lisp, PKCallConv *conv);
PK_RES pk_arity_convert(Pocket lisp, int arity, size_t *output);
PK_RES pk_callconv(Pocket lisp, PKAtom *atom, size_t arity, pk_bool insert_result, PKCallConv *output);
void pk_callconv_quick(void *user_closure, PKFn fn, size_t arity, PKCallConv *output);
PK_RES pk_bind_lambda_list(Pocket lisp, PKAtom *symbols, PKAtomSlice values);

PK_RES pk_slice_list(Pocket lisp, PKAtomSlice atoms, PKAtom **output);
PK_RES pk_slice_list_rev(Pocket lisp, PKAtomSlice atoms, PKAtom **output);
PK_RES pk_slice_list_tailed(Pocket lisp, PKAtomSlice atoms, PKAtom **output);

PK_RES pk_return_push(Pocket lisp);
PK_RES pk_return_insert(Pocket lisp);

PK_RES pk_read_atom_string(PKReader *r, PKAtomString **string);
PK_RES pk_read_atom_number(PKReader *r, PKAtomNumber **number);
PK_RES pk_read_atom_symbol(PKReader *r, PKAtomSymbol **symbol);
PK_RES pk_read_atom_cons(PKReader *r, PKAtom **output);
PK_RES pk_read_atom_simple_macro(PKReader *r, PKAtomSymbol *macro, PKAtomCons **output);
PK_RES pk_read_atom_unquote_macro(PKReader *r, PKAtomCons **output);
PK_RES pk_read_atom(PKReader *r, PKAtom **atom);

PK_RES pk_read_string(Pocket lisp, char *c, size_t length, PK_READ mode, PKAtom **output);

PKAtoms pk_atoms_init(void);
PK_RES pk_atoms_push(Pocket lisp, PKAtoms *atoms, PKAtom *atom, size_t init);
void pk_atoms_free(Pocket lisp, PKAtoms *atoms);

PKBytes pk_bytes_init(void);
PK_RES pk_bytes_push(Pocket lisp, PKBytes *bytes, pk_u8 byte, size_t init);
void pk_bytes_free(Pocket lisp, PKBytes *bytes);

PK_RES pk_compile_evlist(PKCompiler *c, PKAtom *args);
PK_RES pk_compile_special(PKCompiler *c, PKAtomSymbol *symbol, PKAtom *args, pk_bool *is_special);
PK_RES pk_compile_expression(PKCompiler *c, PKAtomCons *expr);
PK_RES pk_compile_value(PKCompiler *c, PKAtom *value);
PK_RES pk_compile_compile(PKCompiler *c, size_t arity, PKAtomLFunc **output);
PK_RES pk_compile_lambda(Pocket lisp, PKAtom *args, PKAtom *body, PKAtomLFunc **output);
PK_RES pk_compile_atom(Pocket lisp, PKAtom *value, PKAtomLFunc **output);

PK_RES pk_lfunc_exec(Pocket lisp, PKAtomLFunc *lfunc);

const char *pk_ident_opcode(pk_u8 op);
const char *pk_ident_atomty(PKAtomTy ty);

PK_OPCODE_TY pk_opcode_ty(pk_u8 op);
    
PK_RES pk_dump_hex_atom(Pocket lisp, PKAtomLFunc *lfunc);
#endif
