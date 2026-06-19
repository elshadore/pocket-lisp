#ifndef POCKET_INTERNALS_H
#define POCKET_INTERNALS_H

#include "pocket.h"

#define pk_cdolist(lisp_, cursor_, list_) \
    for (PKAtom *_pk_tail_ = (list_), *cursor_ = NULL; \
         _pk_tail_->tag.ty == PKAtomTy_Nil ? 0 : \
            (_pk_tail_->tag.ty == PKAtomTy_Cons ? \
                (cursor_ = ((PKAtomCons *)_pk_tail_)->car, 1) : \
                (pk_error(lisp_), 0)); \
         _pk_tail_ = ((PKAtomCons *)_pk_tail_)->cdr)


typedef union PKAtom_ PKAtom;

typedef enum PKAtomTy_ {
    PKAtomTy_Free = 0,
    PKAtomTy_Nil,
    PKAtomTy_Number,
    PKAtomTy_Symbol,
    PKAtomTy_String,
    PKAtomTy_Cons,
    PKAtomTy_CFunc,
} PKAtomTy;

typedef enum PKEnvTy_ {
    PKEnvTy_Var = 0,
    PKEnvTy_Fun,
} PKEnvTy;

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

typedef struct PKFuncArity_ {
    PKArity mode;
    int args;
} PKFuncArity;

typedef struct PKFuncRecord_ {
    PKString sym;
    void *user_closure;
    PKFn fn;
    int args;
    PKArity mode;
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
    PKFuncMode_Macro,
} PKFuncMode;

typedef enum PKFuncTy_ {
    PKFuncTy_CFunc = 0,
    PKFuncTy_Lambda,
    PKFuncTy_Expression,
    PKFuncTy_Evlist,
} PKFuncTy;

typedef struct PKFuncCall_ {
    PKFuncTy ty;
    union {
        struct {
            void *user_closure;
            PKFn fn;
        } c;
        struct {
            PKAtom *args;
            PKAtom *body;
        } lisp;
        PKAtom *value;
    } as;
    PKFuncMode mode;
    size_t final_arity;
    size_t extra_nils;
    bool insert_result;
    PKAtom *expression;
} PKFuncCall;

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
    PKEnvTy ty;
    PKAtomSymbol *symbol;
    PKAtom *restore;
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
    size_t lets_offset;
    PKFuncMode mode;
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

#define PK_SYMTABLE_INIT_CAPACITY (64)

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

#define PK_POOL_MAX (1024)

typedef struct PKPool_ PKPool;
struct PKPool_ {
    PKAtom e[PK_POOL_MAX];
    PKPool *next;
};

typedef struct PKAtoms_ {
    PKAtom **e;
    size_t length;
} PKAtoms;

typedef struct PKCache_ {
    PKAtom *nil;
    PKAtomSymbol *nil_sym;
    PKAtomSymbol *t;
    PKAtomSymbol *lambda;
    PKAtomSymbol *macro;
    PKAtomSymbol *quote;
    PKAtomSymbol *progn;
    PKAtomSymbol *if_sym;
    PKAtomSymbol *while_sym;
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
    PKAtomFree *free;
    PKPool *pool;
};

#define pk_index_inv(i_, length_) ((length_) - (i_) - 1)

size_t pk_grow_capacity(size_t old_capacity, size_t init_capacity);
void pk_print(Pocket lisp, char *c, size_t length);
void pk_puts(Pocket lisp, char *c, size_t length);

PKAtom *pk_atom_alloc(Pocket lisp);
void pk_atom_free(Pocket lisp, PKAtom *atom);
PKAtomNumber *pk_atom_int(Pocket lisp, int value);
PKAtomNumber *pk_atom_float(Pocket lisp, float value);
PKAtomString *pk_atom_string(Pocket lisp, PKString string);
PKAtomString *pk_atom_string_nomemcpy(Pocket lisp, PKString string);
PKAtomSymbol *pk_atom_symbol_uninterned(Pocket lisp, PKString id);
PKAtomSymbol *pk_atom_symbol_interned(Pocket lisp, PKString id);
PKAtomCons *pk_atom_cons(Pocket lisp, PKAtom *car, PKAtom *cdr);
PKAtomCFunc *pk_atom_cfunc(Pocket lisp, void *user_closure, PKFn fn, PKFuncArity arity);
PKAtom *pk_atom_nil(Pocket lisp);

PKAtomNumber *pk_atom_cast_number(Pocket lisp, PKAtom *atom);
PKAtomString *pk_atom_cast_string(Pocket lisp, PKAtom *atom);
PKAtomSymbol *pk_atom_cast_symbol(Pocket lisp, PKAtom *atom);
PKAtomCons *pk_atom_cast_cons(Pocket lisp, PKAtom *atom);
PKAtomCFunc *pk_atom_cast_cfunc(Pocket lisp, PKAtom *atom);

bool pk_atom_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
bool pk_atom_symbol_eq(Pocket lisp, PKAtomSymbol *lhs, PKAtomSymbol *rhs);
bool pk_atom_string_eq(Pocket lisp, PKAtomString *lhs, PKAtomString *rhs);

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
PKAtom *pk_stack_pop(Pocket lisp);
PKAtom *pk_stack_get(Pocket lisp, int stack_pointer);
PKAtoms pk_stack_slice(Pocket lisp);
void pk_stack_set(Pocket lisp, int stack_pointer, PKAtom *atom);
size_t pk_stack_total(Pocket lisp);

void pk_error_impl(Pocket lisp, const char *file, int line);
#define pk_error(lisp) pk_error_impl(lisp, __FILE__, __LINE__)

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
void pk_writer_address(PKWriter *w, uintptr_t address);

uint8_t pk_char_to_digit(char c);
bool pk_char_is_hex(char c);
uint8_t pk_char_to_hex(char c);
char pk_char_from_hex(uint8_t byte);
bool pk_char_is_digit(char c);
char pk_char_from_digit(uint8_t integer);
bool pk_char_is_whitespace(char c);
bool pk_char_is_alphabet(char c);
bool pk_char_is_symbol(char c);

PKAtom *pk_read_atom(PKReader *r);
PKAtomCons *pk_read_from_string(Pocket lisp, PKString string);

void pk_frame_push(Pocket lisp, size_t arity, PKFuncMode mode);
void pk_frame_pop(Pocket lisp);
void pk_frame_clear(Pocket lisp);
void pk_let_push(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom *value);
void pk_let_pop(Pocket lisp, size_t n);

size_t pk_hash_djb2(char *c, size_t length);
size_t pk_hash_pointer(void *ptr);

void pk_symtable_grow(Pocket lisp, PKSymTable *st);
PKAtom *pk_symtable_put(Pocket lisp, PKSymTable *st, PKAtomSymbol *key, PKAtom *value);
PKAtom *pk_symtable_get(Pocket lisp, PKSymTable *st, PKAtomSymbol *key);
PKAtom *pk_symtable_rem(Pocket lisp, PKSymTable *st, PKAtomSymbol *key);
PKAtom *pk_symtable_alist(Pocket lisp, PKSymTable *st);
void pk_symtable_deinit(Pocket lisp, PKSymTable *st);

PKAtom *pk_env_set(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom *value);
PKAtom *pk_env_get(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym);
PKAtom *pk_env_unbind(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym);

void pk_gc_collect(Pocket lisp);
void pk_load_std(Pocket lisp);

void pk_atom_eval(Pocket lisp, PKAtom *atom);
void pk_atom_evlist(Pocket lisp, PKAtom *list);
size_t pk_atom_evrec(Pocket lisp, PKAtom *list);

PKString pk_slurp(Pocket lisp, const char *file_path);
PKAtom *pk_get_result(Pocket lisp);

void pk_call(Pocket lisp, PKFuncCall call);
PKFuncCall pk_get_callconv(Pocket lisp, PKAtom *atom, int arity);

#endif
