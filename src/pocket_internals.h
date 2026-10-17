#ifndef POCKET_LISP_INTERNALS_H
#define POCKET_LISP_INTERNALS_H

#include "pocket.h"

typedef union PKAtomData_ PKAtom;

typedef enum PK_SYMTY_ {
    PK_SYMTY_SYMBOL = 0,
    PK_SYMTY_KEYWORD,
} PK_SYMTY;

typedef struct PKAtoms_ {
    PKAtom **e;
    size_t length;
} PKAtoms;

#define PK_ATOMS_EMPTY (PKAtoms){.e = NULL, .length = 0}

#define PK_ARRAY_EMPTY (PKArray){.e = NULL, .count = 0, .capacity = 0}

typedef struct PKArray_ {
    PKAtom **e;
    size_t count;
    size_t capacity;
} PKArray;

#define PK_BYTES_EMPTY (PKBytes){.e = NULL, .count = 0, .capacity = 0}

typedef struct PKBytes_ {
    pk_u8 *e;
    size_t count;
    size_t capacity;
} PKBytes;

#define PK_HASHMAP_SLOT_UNOCCUPIED (0)
#define PK_HASHMAP_SLOT_OCCUPIED (1)
#define PK_HASHMAP_SLOT_DELETED (2)

typedef struct PKHashmapSlot_ {
    PKAtom *key;
    PKAtom *value;
    pk_u8 state;
} PKHashmapSlot;

#define PK_HASHMAP_EMPTY (PKHashmap){.e = NULL, .count = 0, .capacity = 0}

typedef struct PKHashmap_ {
    PKHashmapSlot *e;
    size_t count;
    size_t capacity;
} PKHashmap;

#define PK_SLOTMAP_SLOT_UNOCCUPIED (0)
#define PK_SLOTMAP_SLOT_OCCUPIED (1)

typedef struct PKSlotmapSlot_ {
    PKAtom *symbol;
    size_t index;
    pk_u8 state;
} PKSlotmapSlot;

#define PK_SLOTMAP_EMPTY (PKSlotmap){.e = NULL, .count = 0, .capacity = 0}

typedef struct PKSlotmap_ {
    PKSlotmapSlot *e;
    size_t count;
    size_t capacity;
} PKSlotmap;

#define PK_WRITER_EMPTY (PKWriter){.c = NULL, .count = 0, .capacity = 0}

typedef struct PKWriter_ {
    char *c;
    size_t count;
    size_t capacity;
} PKWriter;

typedef struct PKReader_ {
    PKStr str;
    size_t curr;
    size_t line;
    size_t bol;
    char c;
} PKReader;

typedef struct PKCompiler_ {
    PKBytes bc;
    PKArray atoms;
    PK_CALL type;
    pk_u8 addr;
    size_t protect_depth;
} PKCompiler;

typedef struct PKVM_ {
    PKAtom *lfun;
    size_t curr;
    size_t save;
    pk_u8 op;
    pk_u8 opcode;
    bool finished;
    bool protected;
    size_t protect_offset;
} PKVM;

typedef struct PKAccum_ {
    Pocket lisp;
    PKAtom *head;
    PKAtom *tail;
} PKAccum;

#define PK_CALLCONV_EMPTY (PKCallConv){.args = 0, .variadic_count = 0, .variadic = false, .push_extra_nil = false}

typedef struct PKCallConv_ {
    pk_u8 args;
    pk_u8 variadic_count;
    bool variadic;
    bool push_extra_nil;
} PKCallConv;

#define PK_ATOMTY_ILLEGAL (0)
#define PK_ATOMTY_FREE (1)
#define PK_ATOMTY_UNUSED_1 (2)
#define PK_ATOMTY_UNUSED_2 (3)
#define PK_ATOMTY_UNUSED_3 (4)
#define PK_ATOMTY_UNDEFINED (5)
#define PK_ATOMTY_NIL (6)
#define PK_ATOMTY_TRUE (7)
#define PK_ATOMTY_NUMBER (8)
#define PK_ATOMTY_STRING (9)
#define PK_ATOMTY_SYMBOL (10)
#define PK_ATOMTY_KEYWORD (11)
#define PK_ATOMTY_CONS (12)
#define PK_ATOMTY_ARRAY (13)
#define PK_ATOMTY_TABLE (14)
#define PK_ATOMTY_CFUN (15)
#define PK_ATOMTY_LFUN (16)
#define PK_ATOMTY_CMACRO (17)
#define PK_ATOMTY_LMACRO (18)
#define PK_ATOMTY_LOBJECT (19)
#define PK_ATOMTY_COBJECT (20)

#define PK_ATOMFLAG_NONE (0)
#define PK_ATOMFLAG_IMMORTAL (1 << 0)

typedef struct PKAtomHeader_ {
    pk_u8 type;
    pk_u8 flags;
} PKAtomHeader;

typedef struct PKAtomFree_ PKAtomFree;
struct PKAtomFree_ {
    PKAtomHeader header;
    PKAtomFree *next;
};

#define PK_NUMTY_INT (0)
#define PK_NUMTY_FLOAT (1)

typedef struct PKAtomNumber_ {
    PKAtomHeader *header;
    pk_u8 num_type;
    union {
        int i;
        float f;
    } as;
} PKAtomNumber;

typedef struct PKAtomString_ {
    PKAtomHeader header;
    PKStr str;
} PKAtomString;

#define PK_TYPETY_NOTYPE (0)
#define PK_TYPETY_PRIMITIVE (1)
// Who owns the memory for the Object, (Us we free, User they free (create a destroy method))
#define PK_TYPETY_C_OWNED (2)
#define PK_TYPETY_C_USER (3)
#define PK_TYPETY_LISP (4)

typedef struct PKAtomSymbol_ {
    PKAtomHeader header;
    pk_u8 type_type;
    PKAtom *var;
    PKAtom *fun;
    PKAtom *prop;
    PKAtom *name;
    PKSlotmap slots;
    PKHashmap methods;
    size_t owned_memory;
} PKAtomSymbol;

typedef struct PKAtomKeyword_ {
    PKAtomHeader header;
    PKStr id;
} PKAtomKeyword;

typedef struct PKAtomCons_ {
    PKAtomHeader header;
    PKAtom *car;
    PKAtom *cdr;
} PKAtomCons;

#define PK_INT_ARITY_NORMAL (0)
#define PK_INT_ARITY_OPTIONAL (1)
#define PK_INT_ARITY_VARIADIC (2)

#define PK_CFUNC_ARG_0 (0)
#define PK_CFUNC_ARG_1 (1)
#define PK_CFUNC_ARG_2 (2)
#define PK_CFUNC_ARG_3 (3)

typedef struct PKAtomCFun_ {
    PKAtomHeader header;
    pk_u8 arity;
    pk_u8 args;
    pk_u8 option_start;
    void *user_closure;
    PKFun *fun;
} PKAtomCFun;

#define PK_OP_ILLEGAL (0)
#define PK_OP_NOP (1)
#define PK_OP_RET (2)
#define PK_OP_LOAD (3)
#define PK_OP_LOAD_NIL (4)
#define PK_OP_LOAD_TRUE (5)
#define PK_OP_CALL (6)
#define PK_OP_BLOCK_BEGIN (7)
#define PK_OP_BLOCK_END (8)
#define PK_OP_BLOCK_CLEAR (9)
#define PK_OP_JMP_UNLESS (10)
#define PK_OP_JMP (11)
#define PK_OP_JMP_BACK (12)
#define PK_OP_SET_VAR (13)
#define PK_OP_SET_FUN (14)
#define PK_OP_SET_PROP (15)
#define PK_OP_LET_VAR (16)
#define PK_OP_LET_FUN (17)
#define PK_OP_LET_PROP (18)
#define PK_OP_GET_VAR (19)
#define PK_OP_GET_FUN (20)
#define PK_OP_GET_PROP (21)
#define PK_OP_MAKE_LIST (22)
#define PK_OP_MAKE_LIST_PACKED (23)
#define PK_OP_MERGE_LISTS (24)
#define PK_OP_STRCAT (25)
#define PK_OP_JMP_WHEN (26)
#define PK_OP_DIG (27)
#define PK_OP_CAR (28)
#define PK_OP_CAR_STRICT (29)
#define PK_OP_CDR (30)
#define PK_OP_CDR_STRICT (31)
#define PK_OP_PROTECT_BEGIN (32)
#define PK_OP_PROTECT_END (33)
#define PK_OP_BLOCK_POP (34)

typedef enum PK_OPCODE_TY_ {
    PK_OPCODE_TY_NORMAL = 0,
    PK_OPCODE_TY_LOAD,
    PK_OPCODE_TY_LIT
} PK_OPCODE_TY;

typedef struct PKAtomLFun_ {
    PKAtomHeader header;
    pk_u8 args;
    pk_u8 arity;
    pk_u8 option_start;
    struct {
        pk_u8 *e;
        size_t length;
    } bc;
    PKAtoms atoms;
} PKAtomLFun;

typedef struct PKAtomCObject_ {
    PKAtomHeader header;
    PKAtom *type;
    void *data;
} PKAtomCObject;

typedef struct PKAtomLObject_ {
    PKAtomHeader *header;
    PKAtom *type;
    PKAtoms *slots;
} PKAtomLObject;

typedef union PKAtomData_ {
    PKAtomHeader header;
    PKAtomFree free;
    PKAtomNumber number;
    PKAtomString string;
    PKAtomSymbol symbol;
    PKAtomKeyword keyword;
    PKAtomCons cons;
    PKAtomCFun cfun;
    PKAtomLFun lfun;
    PKAtomCFun cmacro;
    PKAtomLFun lmacro;
    PKAtomCObject cobject;
    PKAtomLObject lobject;
} PKAtomData;

#define PK_FRAME_EMPTY (PKFrame){0}

typedef struct PKFrame_ {
    size_t stack_offset;
    size_t let_offset;
} PKFrame;

#define PK_FRAMES_EMPTY (PKFrames){.e = NULL, .count = 0, .capacity = 0}

typedef struct PKFrames {
    PKFrame *e;
    size_t count;
    size_t capacity;
} PKFrames;

typedef struct PKLet_ {
    PK_ENV env;
    PKAtom *symbol;
    PKAtom *restore;
} PKLet;

#define PK_LETS_EMPTY (PKLets){.e = NULL, .count = 0, .capacity = 0}

typedef struct PKLets_ {
    PKLet *e;
    size_t count;
    size_t capacity;
} PKLets;

#define PK_POOL_MAX (1024)
typedef struct PKPool_ PKPool;

struct PKPool_ {
    PKAtomData e[PK_POOL_MAX];
    PKPool *next;
};

#define PK_CACHE_EMPTY (PKCache) { \
    .atom_true = NULL, \
    .nil = NULL, \
    .undefined = NULL, \
    \
    .empty_string = NULL, \
    \
    .lambda = NULL, \
    .macro = NULL, \
    \
    .progn = NULL, \
    .sym_if = NULL, \
    .sym_while = NULL, \
    .error = NULL, \
    .throw = NULL, \
    .var = NULL, \
    .fvar = NULL, \
    .let = NULL, \
    .let_star = NULL, \
    .flet = NULL, \
    .flet_star = NULL, \
    .block = NULL, \
    .apply = NULL, \
    .macro_apply = NULL, \
    \
    .quote = NULL, \
    .quasiquote = NULL, \
    .unquote = NULL, \
    .unquote_splice = NULL, \
    .string_substitute = NULL, \
}

typedef struct PKCache_ {
    PKAtom *atom_true;
    PKAtom *nil;
    PKAtom *undefined;

    PKAtomString *empty_string;
    
    PKAtomSymbol *lambda;
    PKAtomSymbol *macro;
    
    PKAtomSymbol *progn;
    PKAtomSymbol *sym_if;
    PKAtomSymbol *sym_while;
    PKAtomSymbol *sym_and;
    PKAtomSymbol *sym_or;
    PKAtomSymbol *error;
    PKAtomSymbol *throw;
    PKAtomSymbol *var;
    PKAtomSymbol *fvar;
    PKAtomSymbol *let;
    PKAtomSymbol *let_star;
    PKAtomSymbol *protect;
    PKAtomSymbol *unwind_protect;
    PKAtomSymbol *flet;
    PKAtomSymbol *flet_star;
    PKAtomSymbol *block;
    PKAtomSymbol *apply;
    PKAtomSymbol *macro_apply;
    
    PKAtomSymbol *quote;
    PKAtomSymbol *quasiquote;
    PKAtomSymbol *unquote;
    PKAtomSymbol *unquote_splice;
    PKAtomSymbol *string_substitute;
    
} PKCache;

#define POCKET_LISP_MACHINE_EMPTY (PocketLispMachine) { \
    .stack = PK_ARRAY_EMPTY, \
    .lets = PK_LETS_EMPTY, \
    .frames = PK_FRAMES_EMPTY, \
    .symbolizer = PK_HASHMAP_EMPTY, \
    .current_frame = PK_FRAME_EMPTY, \
    .pool = NULL, \
    .free = NULL, \
    .cache = PK_CACHE_EMPTY, \
    .initialized = false, \
}

typedef struct PocketLispMachine_ {
    PKArray stack;
    PKLets lets;
    PKFrames frames;
    PKFrame current_frame;
    PKHashmap symbolizer;
    PKPool *pool;
    PKAtomFree *free;
    PKCache cache;
    bool initialized;
} PocketLispMachine;

size_t pk_grow_capacity(size_t old_capacity, size_t init_capacity);
    
void *pk_malloc(Pocket lisp, size_t size);
void *pk_realloc(Pocket lisp, void *ptr, size_t new_size);
void pk_free(Pocket lisp, void *ptr);

void pk_pool_expand(Pocket lisp);

PKAtom *pk_atom_alloc(Pocket lisp);
void pk_atom_free(Pocket lisp, PKAtom *atom);

bool pk_atom_eq(PKAtom *lhs, PKAtom *rhs);
size_t pk_atom_hash(PKAtom *atom);

bool pk_atom_is_nil(PKAtom *atom);
bool pk_atom_is_true(PKAtom *atom);
bool pk_atom_is_cons(PKAtom *atom);
bool pk_atom_is_string(PKAtom *atom);
bool pk_atom_is_symbol(PKAtom *atom);
bool pk_atom_is_keyword(PKAtom *atom);

PKAtomCons *pk_atom_assert_cons(Pocket lisp, PKAtom *atom);
PKAtomString *pk_atom_assert_number(Pocket lisp, PKAtom *atom);
PKAtomString *pk_atom_assert_string(Pocket lisp, PKAtom *atom);
PKAtomSymbol *pk_atom_assert_symbol(Pocket lisp, PKAtom *atom);
PKAtomKeyword *pk_atom_assert_keyword(Pocket lisp, PKAtom *atom);

PKAtomString *pk_atom_string_nomemcpy(Pocket lisp, PKStr string);
PKAtomString *pk_atom_string(Pocket lisp, PKStr string);
PKAtomString *pk_atom_string_alloc(Pocket lisp, size_t length);
PKAtomString *pk_atom_stringc(Pocket lisp, char *cstring);
PKAtomString *pk_atom_string_cat(Pocket lisp, PKAtoms atoms);
bool pk_atom_string_cmp(PKAtomString *string, PKStr comp);

PKAtomCons *pk_atom_cons(Pocket lisp, PKAtom *car, PKAtom *cdr);
PKAtomCons *pk_atom_list1(Pocket lisp, PKAtom *first);
PKAtomCons *pk_atom_list2(Pocket lisp, PKAtom *first, PKAtom *second);
PKAtomCons *pk_atom_list3(Pocket lisp, PKAtom *first, PKAtom *second, PKAtom *third);

PKAtom *pk_lfun_exec(PKAtom *lfun, size_t save);    

void pk_print_str(PKStr string);

void pk_frame_push(Pocket lisp, size_t arity);
void pk_frame_clear(Pocket lisp);
void pk_frame_unwind(Pocket lisp);
size_t pk_frame_length(Pocket lisp);

void pk_stack_push(Pocket lisp, PKAtom *atom);
PKAtom *pk_stack_funcall(PK_CALL call, pk_u8 arity);
PKAtom *pk_stack_head();
PKAtom *pk_stack_pop();
void pk_stack_pop_unchecked(size_t n);
PKAtoms *pk_stack_slice();

PKAtom *pk_stack_slice_down(size_t depth, PKAtoms **output);
PKAtom *pk_stack_op_list(size_t depth);
PKAtom *pk_stack_op_list_packed(size_t depth);
PKAtom *pk_stack_op_list_merge(size_t depth);
PKAtom *pk_stack_op_strcat(size_t depth);
PKAtom *pk_stack_op_dig(size_t depth);
PKAtom *pk_stack_op_car(bool strict);
PKAtom *pk_stack_op_cdr(bool strict);
void pk_stack_reset_to(size_t count);
    
void pk_let_unwind(size_t n);

PKAtom *pk_read_atom(PKReader *r);
PKAtom *pk_read_barred(PKReader *r, PKWriter *w, PK_SYMTY type);
    
void pk_writer_deinit(PKWriter *w);
PKAtom *pk_writer_expand(PKWriter *w, size_t needed);
PKAtom *pk_writer_char(PKWriter *w, char c);
PKAtom *pk_writer_cstr(PKWriter *w, const char *cstr);
PKAtom *pk_writer_string(PKWriter *w, PKStr string);
PKAtom *pk_writer_string_escaped(PKWriter *w, PKStr string);
PK_PRINTF_FMT(2, 3) PKAtom *pk_writer_fmt(PKWriter *w, const char *fmt, ...);
PKAtom *pk_writer_int(PKWriter *w, int integer);
PKAtom *pk_writer_float(PKWriter *w, float floater);
PKAtom *pk_writer_address(PKWriter *w, size_t address);
PKAtom *pk_writer_newline(PKWriter *w);
PKStr pk_writer_slice(PKWriter *w);
void pk_writer_reset(PKWriter *w);
PKAtom *pk_writer_symbol_name(PKWriter *w, PKStr name);
PKAtom *pk_writer_atom(PKWriter *w, PKAtom *atom);
    
PKStr pk_str_alloc(Pocket lisp, size_t length);
PKStr pk_str_dupe(Pocket lisp, PKStr input);
PKStr pk_str_from_cstr(char *cstr);
bool pk_str_eq(PKStr a, PKStr b);
bool pk_str_needs_bars(PKStr string);
void pk_str_reverse_mut(PKStr string);

pk_u8 pk_char_to_digit(char c);
char pk_char_from_digit(pk_u8 integer);
bool pk_char_is_digit(char c);
bool pk_char_is_whitespace(char c);
bool pk_char_is_alphabet(char c);
bool pk_char_is_symbol(char c);
char pk_char_from_hex(pk_u8 byte);
bool pk_char_is_hex(char c);
pk_u8 pk_char_to_hex(char c);

PKAccum pk_accum_new(Pocket lisp);
void pk_accum_push(PKAccum *acc, PKAtom atom);

void pk_array_push(Pocket lisp, PKArray *array, PKAtom *atom, size_t init_capacity);
void pk_array_deinit(Pocket lisp, PKArray *array);
void pk_bytes_push(Pocket lisp, PKBytes *bytes, pk_u8 byte, size_t init_capacity);
void pk_bytes_deinit(Pocket lisp, PKBytes *bytes);
void pk_atoms_free(Pocket lisp, PKAtoms *atoms);
    
PKAtom *pk_hashmap_insert(PKHashmap *map, PKAtom *key, PKAtom *value);
PKAtom *pk_hashmap_get_str(PKHashmap *map, PKStr key);
PKAtom *pk_hashmap_get(PKHashmap *map, PKAtom *key);
PKAtom *pk_hashmap_remove(PKHashmap *map, PKAtom *key);
void pk_hashmap_deinit(PKHashmap *map);

size_t pk_slotmap_index(PKSlotmap *map, PKAtom *symbol);
PKSlotmapSlot *pk_slotmap_find(PKSlotmap *map, PKAtom *symbol);
bool pk_slotmap_index_of(PKSlotmap *map, PKAtom *symbol, size_t *index);
PKAtom *pk_slotmap_insert(PKSlotmap *map, PKAtom *symbol, size_t *index);
PKAtom *pk_slotmap_reserve(PKSlotmap *map, size_t count);
void pk_slotmap_deinit(PKSlotmap *map);
    
size_t pk_hash_djb2(PKStr string);
size_t pk_hash_pointer(void *ptr);

PKAtom *pk_cmp_patch_byte(PKCompiler *c, pk_u8 addr, pk_u8 byte);
PKAtom *pk_cmp_push_byte(PKCompiler *c, pk_u8 byte);
PKAtom *pk_cmp_push_any(PKCompiler *c, size_t value);
PKAtom *pk_cmp_push_atom(PKCompiler *c, PKAtom *atom);
PKAtom *pk_cmp_load_nil(PKCompiler *c);
PKAtom *pk_cmp_load_true(PKCompiler *c);
PKAtom *pk_cmp_load(PKCompiler *c, PKAtom *atom);
    
PKAtom *pk_compile_value(PKCompiler *c, PKAtom *value);
PKAtom *pk_compile_lambda(PK_CALL type, PKAtom *args, PKAtom *body);
PKAtom *pk_compile_atom(PKCompiler *c, PKAtom *value);
PKAtom *pk_compile_strsub(PKCompiler *c, PKStr string);

PKStr pk_ident_opcode(pk_u8 op);
PKStr pk_ident_atomty(pk_u8 ty);
PKStr pk_ident_arity(pk_u8 arity);
PKStr pk_ident_env(PK_ENV env);
PKStr pk_ident_type(PK_TYPE ty);
PK_OPCODE_TY pk_opcode_ty(pk_u8 op);
    
PKAtom *pk_macroexpand1_(PKAtom *atom, bool *expanded);

PKAtom *pk_std_load(void);
void pk_deinit_cache(void);

PKAtom *pk_fs_slurp_str(PKStr file_path);
PKAtom *pk_fs_slurp(PKAtom *file_path);
PKAtom *pk_fs_read_user_input(void);

PKVM pk_vm_init(PKAtom *lfun, size_t save);
PKAtom *pk_vm_exec(PKVM *vm);

#ifdef POCKET_CFLAG_DEBUG
void pk_debug_leakcheck(void);
PKAtom *pk_debug_dump(const char *tag, PK_DUMP opt);
#endif
    
#endif // POCKET_LISP_INTERNALS_H
