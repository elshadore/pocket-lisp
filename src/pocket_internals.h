#ifndef POCKET_INTERNALS_H
#define POCKET_INTERNALS_H

#include "pocket.h"

typedef union PKAtom_ PKAtom;

typedef enum PKAtomTy_ {
    PKAtomTy_Free = 0,
    PKAtomTy_Nil,
    PKAtomTy_Number,
    PKAtomTy_Symbol,
    PKAtomTy_String,
    PKAtomTy_Cons,
} PKAtomTy;

typedef struct PKAtomTag_ {
    PKAtomTy ty;
    bool marked;
} PKAtomTag;

typedef struct PKAtomFree_ PKAtomFree;
struct PKAtomFree_ {
    PKAtomTag tag;
    PKAtomFree *next;
};

typedef enum PKNumberTy_ {
    PKNumberTy_Int = 0,
    PKNumberTy_Float,
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
    PKString lit;
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

union PKAtom_ {
    PKAtomTag tag;
    PKAtomFree free;
    PKAtomNumber number;
    PKAtomSymbol symbol;
    PKAtomString string;
    PKAtomCons cons;
};

typedef struct PKWriter_ {
    Pocket lisp;
    char *c;
    size_t count;
    size_t capacity;
} PKWriter;

#define PK_WRITER_INIT_CAPACITY (256)

typedef struct PKReader_ {
    Pocket lisp;
    PKString src;
    char c;
    size_t curr;
} PKReader;

#define PK_LET_INIT_CAPACITY (256)

typedef struct PKLet_ {
    PKAtomSymbol *symbol;
    PKAtom *value;
} PKLet;

typedef struct PKLets_ {
    PKLet *e;
    size_t count;
    size_t capacity;
} PKLets;

#define PK_STACK_INIT_CAPACITY (128)

typedef struct PKStack_ {
    PKAtom **e;
    size_t count;
    size_t capacity;
} PKStack;

#define PK_FRAMES_INIT_CAPACITY (64)

typedef struct PKFrame_ {
    size_t stack_offset;
    size_t arity;
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

#define PK_ENV_INIT_CAPACITY (64)

typedef struct PKEnvSlot_ PKEnvSlot;
struct PKEnvSlot_ {
    PKAtomSymbol *key;
    PKAtom *value;
    PKEnvSlot *chain;
};

typedef struct PKEnv_ {
    PKEnvSlot **e;
    size_t count;
    size_t capacity;
} PKEnv;

#define PK_POOL_MAX (1024)

typedef struct PKPool_ PKPool;
struct PKPool_ {
    PKAtom e[PK_POOL_MAX];
    PKPool *next;
};

typedef struct PKCache_ {
    PKAtom *nil;
    PKAtomSymbol *nilsym;
    PKAtomSymbol *t;
    PKAtomSymbol *lambda;
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
    PKEnv vars;
    PKEnv funs;
    PKIntern intern;
    PKAtomFree *free;
    PKPool *pool;
};

size_t pk_grow_capacity(size_t old_capacity, size_t init_capacity);
void pk_print(Pocket lisp, char *c, size_t length);

PKAtom *pk_atom_alloc(Pocket lisp);
void pk_atom_free(Pocket lisp, PKAtom *atom);
PKAtomNumber *pk_atom_int(Pocket lisp, int value);
PKAtomNumber *pk_atom_float(Pocket lisp, float value);
PKAtomString *pk_atom_string(Pocket lisp, PKString string);
PKAtomSymbol *pk_atom_symbol_uninterned(Pocket lisp, PKString id);
PKAtomSymbol *pk_atom_symbol_interned(Pocket lisp, PKString id);
PKAtomCons *pk_atom_cons(Pocket lisp, PKAtom *car, PKAtom *cdr);
PKAtom *pk_atom_nil(Pocket lisp);

PKAtomNumber *pk_atom_cast_number(Pocket lisp, PKAtom *atom);
PKAtomString *pk_atom_cast_string(Pocket lisp, PKAtom *atom);
PKAtomSymbol *pk_atom_cast_symbol(Pocket lisp, PKAtom *atom);
PKAtomCons *pk_atom_cast_cons(Pocket lisp, PKAtom *atom);

PKString pk_string_dupe(Pocket lisp, PKString string);
void pk_string_free(Pocket lisp, PKString string);
PKString pk_string_from_cstr(char *cstr);
bool pk_string_eq(PKString a, PKString b);

void *pk_malloc(Pocket lisp, size_t size);
void *pk_realloc(Pocket lisp, void *ptr, size_t old_size, size_t new_size);
void pk_free(Pocket lisp, void *ptr, size_t size);

void pk_push(Pocket lisp, PKAtom *atom);
void pk_stack_expand(Pocket lisp, size_t total);
size_t pk_sp_index(Pocket lisp, int stack_pointer);
PKAtom *pk_stack_get(Pocket lisp, int stack_pointer);
void pk_stack_set(Pocket lisp, int stack_pointer, PKAtom *atom);
size_t pk_stack_total(Pocket lisp);

void pk_error(Pocket lisp);

PKAtomNumber *pk_number_add(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
PKAtomNumber *pk_number_sub(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
PKAtomNumber *pk_number_mul(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
PKAtomNumber *pk_number_div(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
PKAtomNumber *pk_number_mod(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);

bool pk_number_lt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
bool pk_number_lte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
bool pk_number_gt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
bool pk_number_gte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
bool pk_number_eq(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);

int pk_number_to_int(PKAtomNumber *num);
float pk_number_to_float(PKAtomNumber *num);

PKWriter pk_writer_init(Pocket lisp);
void pk_writer_deinit(PKWriter *w);
void pk_writer_char(PKWriter *w, char c);
void pk_writer_cstr(PKWriter *w, char *cstr);
void pk_writer_string(PKWriter *w, PKString string);
void pk_writer_string_escaped(PKWriter *w, PKString string);
void PK_PRINTF(2, 3) pk_writer_printf(PKWriter *w, const char *fmt, ...);
void pk_writer_newline(PKWriter *w);
void pk_writer_int(PKWriter *w, int integer);
void pk_writer_float(PKWriter *w, float floater);
void pk_writer_atom(PKWriter *w, PKAtom *atom);
PKString pk_writer_get(PKWriter *w);
void pk_writer_reset(PKWriter *w);
void pk_writer_print(PKWriter *w);

uint8_t pk_char_to_digit(char c);
char pk_char_from_digit(uint8_t integer);
char pk_char_from_hex(uint8_t byte);
bool pk_char_is_digit(char c);
bool pk_char_is_whitespace(char c);
bool pk_char_is_alphabet(char c);
bool pk_char_is_symbol(char c);

PKAtom *pk_read_atom(PKReader *r);
PKAtomCons *pk_read_from_string(Pocket lisp, PKString string);

void pk_frame_push(Pocket lisp, size_t arity);
void pk_frame_pop(Pocket lisp);

size_t pk_hash_djb2(char *c, size_t length);
size_t pk_hash_pointer(void *ptr);

void pk_env_grow(Pocket lisp, PKEnv *env);
PKAtom *pk_env_put(Pocket lisp, PKEnv *env, PKAtomSymbol *key, PKAtom *value);
PKAtom *pk_env_query(Pocket lisp, PKEnv *env, PKAtomSymbol *key);
PKAtom *pk_env_remove(Pocket lisp, PKEnv *env, PKAtomSymbol *key);
void pk_env_deinit(Pocket lisp, PKEnv *env);

void pk_env_set(Pocket lisp, PKAtomSymbol *sym, PKAtom *value);
PKAtom *pk_env_get(Pocket lisp, PKAtomSymbol *sym);
void pk_env_unbind(Pocket lisp, PKAtomSymbol *sym);

void pk_env_fset(Pocket lisp, PKAtomSymbol *sym, PKAtom *value);
PKAtom *pk_env_fget(Pocket lisp, PKAtomSymbol *sym);
void pk_env_funbind(Pocket lisp, PKAtomSymbol *sym);

#endif
