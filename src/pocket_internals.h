#ifndef POCKET_INTERNALS_H
#define POCKET_INTERNALS_H

#include "pocket.h"

typedef union PKAtom_ PKAtom;

typedef enum PKAtomTy_ {
    PKAtomTy_Nil = 0,
    PKAtomTy_Free,
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
    PKNumber_Int = 0,
    PKNumber_Float,
} PKNumberTy;

typedef struct PKAtomNumber_ {
    PKAtomTag tag;
    PKNumberTy ty;
    union {
        int i;
        float f;
    } as;
} PKAtomNumber;

typedef struct PKAtomSymbol_ {
    PKAtomTag tag;
    PKString id;
} PKAtomSymbol;

typedef struct PKAtomString_ {
    PKAtomTag tag;
    PKString lit;
} PKAtomString;

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

typedef struct PKStack_ {
    PKAtom **e;
    size_t count;
    size_t capacity;
} PKStack;

#define PK_POOL_MAX (1024)

typedef struct PKPool_ PKPool;
struct PKPool_ {
    PKAtom e[PK_POOL_MAX];
    PKPool *next;
};

struct PocketLispMachine_ {
    PKStack stack;
    PKPool *pool;
    PKAtomFree *free;
    void *user_closure;
    PKAllocFn alloc;
};

size_t pk_grow_capacity(size_t old_capacity, size_t init_capacity);

PKAtom *pk_atom_alloc(Pocket lisp);
void pk_atom_free(Pocket lisp, PKAtom *atom);
PKAtomNumber *pk_make_atom_int(Pocket lisp, int value);
PKAtomNumber *pk_make_atom_float(Pocket lisp, float value);
PKAtomString *pk_make_atom_string(Pocket lisp, PKString string);
PKAtomSymbol *pk_make_atom_symbol(Pocket lisp, PKString id);
PKAtomCons *pk_make_atom_cons(Pocket lisp, PKAtom *car, PKAtom *cdr);
PKAtom *pk_make_atom_nil(Pocket lisp);

PKString pk_string_dupe(Pocket lisp, PKString string);
void pk_string_free(Pocket lisp, PKString string);
PKString pk_string_from_cstr(char *cstr);
bool pk_string_eq(PKString a, PKString b);

void *pk_malloc(Pocket lisp, size_t size);
void *pk_realloc(Pocket lisp, void *ptr, size_t old_size, size_t new_size);
void pk_free(Pocket lisp, void *ptr, size_t size);

void pk_push(Pocket lisp, PKAtom *atom);
PKAtom *pk_stack_get(Pocket lisp, int stack_pointer);

void pk_error(Pocket lisp);

PKWriter pk_writer_init(Pocket lisp);
void pk_writer_deinit(PKWriter *w);
void pk_writer_char(PKWriter *w, char c);
void pk_writer_cstr(PKWriter *w, char *cstr);
void pk_writer_string(PKWriter *w, PKString s);
void pk_writer_int(PKWriter *w, int v);
void pk_writer_float(PKWriter *w, float v);
void pk_writer_string_escaped(PKWriter *w, PKString s);
void pk_writer_atom(PKWriter *w, PKAtom *atom);
PKString pk_writer_get(PKWriter *w);
void pk_writer_reset(PKWriter *w);


#endif
