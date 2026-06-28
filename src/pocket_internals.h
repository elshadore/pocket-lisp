#ifndef POCKET_INTERNALS_H
#define POCKET_INTERNALS_H

#include "pocket.h"

#define pk_cdolist(lisp_, cursor_, list_) \
    for (PKAtom *_pk_tail_ = (list_), *cursor_ = NULL; \
         _pk_tail_->tag.ty == PKAtomTy_Nil ? 0 : \
            (_pk_tail_->tag.ty == PKAtomTy_Cons ? \
                (cursor_ = ((PKAtomCons *)_pk_tail_)->car, 1) : \
                ({ pk_error(lisp_); return PK_Yield; 0; })); \
         _pk_tail_ = ((PKAtomCons *)_pk_tail_)->cdr)

#define pk_cdolist_defer(lisp_, cursor_, list_, err_label_) \
    for (PKAtom *_pk_tail_ = (list_), *cursor_ = NULL; \
         _pk_tail_->tag.ty == PKAtomTy_Nil ? 0 : \
            (_pk_tail_->tag.ty == PKAtomTy_Cons ? \
                (cursor_ = ((PKAtomCons *)_pk_tail_)->car, 1) : \
                ({ pk_error(lisp_); goto err_label_; 0; })); \
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

typedef enum PKEvalMode_ {
    PKEvalMode_Root = 0,
    PKEvalMode_Eval,
    PKEvalMode_Evlist,
    PKEvalMode_Evlist_2,
    PKEvalMode_Evargs,
    PKEvalMode_Apply,
    PKEvalMode_Quote,
    PKEvalMode_If,
    PKEvalMode_While,
    PKEvalMode_While_2,
} PKEvalMode;

typedef enum PKEvalFrameTy_ {
    PKEvalFrameTy_Atom = 0,
    PKEvalFrameTy_Tuple,
    PKEvalFrameTy_CFn,
} PKEvalFrameTy;

typedef struct PKTuple_ {
    PKAtom *a;
    PKAtom *b;
} PKTuple;

typedef struct PKFastCall_ {
    void *user_closure;
    PKFn fn;
} PKFastCall;

typedef union PKFrameData_ {
    PKAtom *atom;
    PKAtomCons *cons;
    PKTuple t;
    PKFastCall fcall;
} PKFrameData;

typedef struct PKFrame_ {
    size_t stack_offset;
    size_t arity;
    size_t lets_offset;
    PKEvalMode mode;
    bool quasiquote;
    bool macro;
    PKFrameData as;
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

typedef enum PKOrder_ {
    PKOrder_Normal = 0,
    PKOrder_Reversed,
} PKOrder;

typedef struct PKStackSlice_ {
    PKOrder order;
    PKAtoms slice;
} PKStackSlice;

#define PK_ARENA_PAGE (1024 * 1024)

typedef struct PKArena_ {
    void *e;
    size_t head;
    size_t size;
} PKArena;

#define PK_ARENA_STACK_INIT_CAPACITY (8)

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
    PKArenaStack arena_stack;
    PKArena arena;
    PKAtomFree *free;
    PKPool *pool;
};

#define pk_index_inv(i_, length_) ((length_) - (i_) - 1)

#define pk_talloc(type_, lisp_, output_) \
pk_malloc(lisp_, sizeof(type_), (void **)(output_))

#define pk_tallocn(type_, lisp_, count_, output_) \
pk_malloc(lisp_, sizeof(type_) * count_, output_)

#define pk_trealloc(type_, lisp_, ptr_, old_count_, new_count_, output_) \
pk_realloc(lisp_, ptr_, sizeof(type_) * old_count_, sizeof(type_) * new_count_, output_)

#define pk_error(lisp) pk_error_impl(lisp, __FILE__, __LINE__)

size_t pk_grow_capacity(size_t old_capacity, size_t init_capacity);
size_t pk_next_pow2(size_t value);

PKRes pk_print(Pocket lisp, char *c, size_t length);
PKRes pk_puts(Pocket lisp, char *c, size_t length);

PKRes pk_atom_alloc(Pocket lisp, PKAtom **output);
void pk_atom_free(Pocket lisp, PKAtom *atom);

PKRes pk_atom_int(Pocket lisp, int value, PKAtomNumber **output);
PKRes pk_atom_float(Pocket lisp, float value, PKAtomNumber **output);
PKRes pk_atom_string(Pocket lisp, PKString string, PKAtomString **output);
PKRes pk_atom_string_nomemcpy(Pocket lisp, PKString string, PKAtomString **output);
PKRes pk_atom_symbol_uninterned(Pocket lisp, PKString id, PKAtomSymbol **output);
PKRes pk_atom_symbol_interned(Pocket lisp, PKString id, PKAtomSymbol **output);
PKRes pk_atom_cons(Pocket lisp, PKAtom *car, PKAtom *cdr, PKAtomCons **output);
PKRes pk_atom_cons_car(Pocket lisp, PKAtom *car, PKAtomCons **output);
PKRes pk_atom_cfunc(Pocket lisp, void *user_closure, PKFn fn, PKFuncArity arity, PKAtomCFunc **output);
PKRes pk_atom_nil_new(Pocket lisp, PKAtom **output);

PKAtom *pk_atom_nil(Pocket lisp);
PKAtom *pk_atom_t(Pocket lisp);

PKRes pk_atom_cast_number(Pocket lisp, PKAtom *atom, PKAtomNumber **output);
PKRes pk_atom_cast_string(Pocket lisp, PKAtom *atom, PKAtomString **output);
PKRes pk_atom_cast_symbol(Pocket lisp, PKAtom *atom, PKAtomSymbol **output);
PKRes pk_atom_cast_cons(Pocket lisp, PKAtom *atom, PKAtomCons **output);
PKRes pk_atom_cast_cfunc(Pocket lisp, PKAtom *atom, PKAtomCFunc **output);

bool pk_atom_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
bool pk_atom_symbol_eq(Pocket lisp, PKAtomSymbol *lhs, PKAtomSymbol *rhs);
bool pk_atom_string_eq(Pocket lisp, PKAtomString *lhs, PKAtomString *rhs);
bool pk_atom_is_true(PKAtom *atom);
bool pk_atom_is_nil(PKAtom *atom);
bool pk_atom_is_symbol(PKAtom *atom);

PKRes pk_string_dupe(Pocket lisp, PKString string, PKString *output);
void pk_string_free(Pocket lisp, PKString string);
PKRes pk_string_from_cstr(char *cstr, PKString *output);
bool pk_string_eq(PKString a, PKString b);

PKRes pk_atom_cons_tail(Pocket lisp, PKAtomCons *cons, PKAtomCons **output);

PKRes pk_malloc(Pocket lisp, size_t size, void **output);
PKRes pk_realloc(Pocket lisp, void *ptr, size_t old_size, size_t new_size, void **output);

void pk_free(Pocket lisp, void *ptr, size_t size);

PKRes pk_push(Pocket lisp, PKAtom *atom);
PKRes pk_stack_expand(Pocket lisp, size_t total);
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

PKRes pk_number_lt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, bool *output);
PKRes pk_number_lte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, bool *output);
PKRes pk_number_gt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, bool *output);
PKRes pk_number_gte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, bool *output);
PKRes pk_number_eq(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs, bool *output);

int pk_number_to_int(PKAtomNumber *num);
float pk_number_to_float(PKAtomNumber *num);

PKWriter pk_writer_init(Pocket lisp);
PKRes pk_writer_deinit(PKWriter *w);
PKRes pk_writer_char(PKWriter *w, char c);
PKRes pk_writer_cstr(PKWriter *w, char *cstr);
PKRes pk_writer_string(PKWriter *w, PKString string);
PKRes pk_writer_string_escaped(PKWriter *w, PKString string);
PKRes PK_PRINTF(2, 3) pk_writer_printf(PKWriter *w, const char *fmt, ...);
PKRes pk_writer_newline(PKWriter *w);
PKRes pk_writer_int(PKWriter *w, int integer);
PKRes pk_writer_float(PKWriter *w, float floater);
PKRes pk_writer_atom(PKWriter *w, PKAtom *atom);
PKRes pk_writer_get(PKWriter *w, PKString *output);
PKRes pk_writer_reset(PKWriter *w);
PKRes pk_writer_print(PKWriter *w);
PKRes pk_writer_address(PKWriter *w, uintptr_t address);

uint8_t pk_char_to_digit(char c);
bool pk_char_is_hex(char c);
uint8_t pk_char_to_hex(char c);
char pk_char_from_hex(uint8_t byte);
bool pk_char_is_digit(char c);
char pk_char_from_digit(uint8_t integer);
bool pk_char_is_whitespace(char c);
bool pk_char_is_alphabet(char c);
bool pk_char_is_symbol(char c);

PKRes pk_read_atom(PKReader *r, PKAtom **output);
PKRes pk_read_from_string(Pocket lisp, PKString string, PKAtom **output);

// Push a frame to the stack, the *arity* param refers to the number of variables to take
// from the previous frame.
PKRes pk_frame_push(Pocket lisp, size_t arity, PKEvalMode mode, PKFrameData data);
// Pop the current frame, clearing all stack allocated variables.
PKRes pk_frame_pop_clear(Pocket lisp);
// Pop the current frame, returning all the current stack allocated variables.
PKRes pk_frame_pop_return(Pocket lisp);
// Clear the current frame of all stack allocated variables.
void pk_frame_clear(Pocket lisp);
size_t pk_frame_length(Pocket lisp);
size_t pk_frame_savepoint(Pocket lisp);
PKAtoms pk_frame_slice(Pocket lisp, PKFrame *frame, size_t length);
PKRes pk_let_push(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom *value);
PKRes pk_let_pop(Pocket lisp, size_t n);

size_t pk_hash_djb2(char *c, size_t length);
size_t pk_hash_pointer(void *ptr);

PKRes pk_symtable_grow(Pocket lisp, PKSymTable *st);
PKRes pk_symtable_put(Pocket lisp, PKSymTable *st, PKAtomSymbol *key, PKAtom *value, PKAtom **output);
PKRes pk_symtable_get(Pocket lisp, PKSymTable *st, PKAtomSymbol *key, PKAtom **output);
PKRes pk_symtable_rem(Pocket lisp, PKSymTable *st, PKAtomSymbol *key, PKAtom **output);
PKRes pk_symtable_alist(Pocket lisp, PKSymTable *st, PKAtom **output);
void pk_symtable_deinit(Pocket lisp, PKSymTable *st);

PKRes pk_env_set(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom *value, PKAtom **output);
PKRes pk_env_get(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom **output);
PKRes pk_env_unbind(Pocket lisp, PKEnvTy ty, PKAtomSymbol *sym, PKAtom **output);

PKRes pk_gc_collect(Pocket lisp);
PKRes pk_load_std(Pocket lisp);

PKRes pk_slurp(Pocket lisp, const char *file_path, PKString *output);

PKRes pk_call(Pocket lisp, PKAtom *atom, size_t arity);
// PKFuncCall pk_get_callconv(Pocket lisp, PKAtom *atom, int arity);

PKRes pk_slice_list(Pocket lisp, PKAtoms atoms, PKAtom **output);
PKRes pk_slice_list_rev(Pocket lisp, PKAtoms atoms, PKAtom **output);

void pk_arena_deinit(Pocket lisp, PKArena *arena);
void pk_arena_deinit_all(Pocket lisp);
PKRes pk_arena_alloc(Pocket lisp, size_t size, void **output);
void *pk_arena_savepoint(Pocket lisp);
void pk_arena_restore(Pocket lisp, void *ptr);

PKRes pk_interp(Pocket lisp, size_t stop);
PKRes pk_interp_eval(Pocket lisp);
PKRes pk_interp_apply(Pocket lisp);
PKRes pk_interp_evlist(Pocket lisp);
PKRes pk_interp_evlist_2(Pocket lisp);
PKRes pk_interp_evargs(Pocket lisp);
PKRes pk_interp_quote(Pocket lisp);
PKRes pk_interp_if(Pocket lisp);
PKRes pk_interp_while(Pocket lisp);
PKRes pk_interp_while_2(Pocket lisp);
PKRes pk_interp_special_form(Pocket lisp, PKAtomSymbol *symbol, PKAtom *rest, PKAtom *expression, bool *is_special);

PKRes pk_ret_top(Pocket lisp);
PKRes pk_ret_nil(Pocket lisp);
PKRes pk_ret_this(Pocket lisp, PKAtom *atom);
PKRes pk_ret_all(Pocket lisp);
PKRes pk_ret_none(Pocket lisp);

PKString pk_ident_evalmode(PKEvalMode mode);
PKEvalFrameTy pk_evalmode_framety(PKEvalMode mode);

#endif
