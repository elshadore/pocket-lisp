#ifndef POCKET_LISP_INTERNALS_H
#define POCKET_LISP_INTERNALS_H

#include "../include/pocket.h"
#include <setjmp.h>

#define PK_FALSE (0)
#define PK_TRUE (1)

typedef union PKAtomData_ PKAtom;

typedef enum PK_NUMBER_CONV_ {
    PK_NUMBER_CONV_FLOAT_CONV = 0,
    PK_NUMBER_CONV_STRICT = 1,
} PK_NUMBER_CONV;

typedef enum PK_SYMTY_ {
    PK_SYMTY_SYMBOL = 0,
    PK_SYMTY_KEYWORD,
} PK_SYMTY;

#define PK_ATOMS_EMPTY (PKAtoms){.e = NULL, .length = 0}

typedef struct PKAtoms_ {
    PKAtom **e;
    size_t length;
} PKAtoms;

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

typedef enum PK_HASHMAP_INSERT_ {
    PK_HASHMAP_INSERT_NORMAL = 0,
    PK_HASHMAP_INSERT_UNIQUE = 1,
} PK_HASHMAP_INSERT;

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

#define PK_WRITER_EMPTY (PKWriter) {.e = NULL, .count = 0, .capacity = 0}

typedef struct PKWriter_ {
    pk_u8 *e;
    size_t count;
    size_t capacity;
} PKWriter;

typedef struct PKParser_ {
    PKStr str;
    size_t curr;
    size_t line;
    size_t bol;
    char c;
} PKParser;

typedef struct PKCompiler_ {
    PKBytes bc;
    PKArray atoms;
    PK_CALL type;
    pk_u8 addr;
    bool transferred;
} PKCompiler;

typedef struct PKDSB_ {
    pk_u8 args;
    pk_u8 lets;
    pk_u8 arity;
    pk_u8 option;
} PKDSB;

typedef struct PKVM_ {
    PKAtom *lfun;
    size_t curr;
    size_t save;
    pk_u8 op;
    pk_u8 opcode;
    bool finished;
} PKVM;

typedef struct PKAccum_ {
    Pocket lisp;
    PKAtom *head;
    PKAtom *tail;
} PKAccum;

typedef enum PK_FUNTY_ {
    PK_FUNTY_C = 0,
    PK_FUNTY_LISP,
} PK_FUNTY;

#define PK_CALLCONV_EMPTY (PKCallConv){.args = 0, .variadic_count = 0, .extra_nils = 0}

typedef struct PKCallConv_ {
    pk_u8 args;
    pk_u8 variadic_count;
    pk_u8 extra_nils;
} PKCallConv;

#define PK_ATOMTY_ILLEGAL (0)
#define PK_ATOMTY_FREE (1)
#define PK_ATOMTY_DESTROYED (2)
#define PK_ATOMTY_UNDEFINED (3)
#define PK_ATOMTY_NIL (4)
#define PK_ATOMTY_TRUE (5)
#define PK_ATOMTY_NUMBER (6)
#define PK_ATOMTY_STRING (7)
#define PK_ATOMTY_SYMBOL (8)
#define PK_ATOMTY_KEYWORD (9)
#define PK_ATOMTY_CONS (10)
#define PK_ATOMTY_ARRAY (11)
#define PK_ATOMTY_OBJECT (12)
#define PK_ATOMTY_CFUN (13)
#define PK_ATOMTY_LFUN (14)
#define PK_ATOMTY_CMACRO (15)
#define PK_ATOMTY_LMACRO (16)
#define PK_ATOMTY_BLOB (17)

typedef enum PKAtomKind_ {
    PK_ATOMKIND_VALUE = 0,
    PK_ATOMKIND_OBJECT = 1,
} PKAtomKind;

#define PK_ATOMFLAG_NONE (0)
#define PK_ATOMFLAG_MARKED (1 << 0)
#define PK_ATOMFLAG_IMMORTAL (1 << 1)

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

typedef struct PKNum_ {
    pk_u8 num_type;
    union {
        int i;
        float f;
    } as;
} PKNum;

typedef struct PKAtomNumber_ {
    PKAtomHeader header;
    PKNum num;
} __attribute__((aligned(sizeof(void *)))) PKAtomNumber;

typedef struct PKAtomString_ {
    PKAtomHeader header;
    PKStr str;
} PKAtomString;

typedef struct PKAtomSymbol_ {
    PKAtomHeader header;
    PKAtomString *name;
    PKAtom *var;
    PKAtom *fun;
    PKHashmap props;
} PKAtomSymbol;

typedef struct PKAtomKeyword_ {
    PKAtomHeader header;
    PKAtomString *name;
} PKAtomKeyword;

typedef struct PKAtomCons_ {
    PKAtomHeader header;
    PKAtom *car;
    PKAtom *cdr;
} PKAtomCons;

#define PK_INT_ARITY_NORMAL (0)
#define PK_INT_ARITY_OPTIONAL (1)
#define PK_INT_ARITY_VARIADIC (2)

typedef struct PKArity_ {
    pk_u8 type;
    pk_u8 args;
    pk_u8 options;
} PKArity;

typedef struct PKAtomCFun_ {
    PKAtomHeader header;
    PKArity arity;
    void *user_closure;
    PKFun fun;
    PKHashmap props;
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
#define PK_OP_LET (15)
#define PK_OP_FLET (16)
#define PK_OP_FETCH (17)
#define PK_OP_FFETCH (18)
#define PK_OP_MAKE_LIST (19)
#define PK_OP_MAKE_LIST_PACKED (20)
#define PK_OP_MERGE_LISTS (21)
#define PK_OP_STRCAT (22)
#define PK_OP_JMP_WHEN (23)
#define PK_OP_DIG (24)
#define PK_OP_CAR (25)
#define PK_OP_CAR_STRICT (26)
#define PK_OP_CDR (27)
#define PK_OP_CDR_STRICT (28)
#define PK_OP_PROTECT_BEGIN (29)
#define PK_OP_PROTECT_END (30)
#define PK_OP_BLOCK_POP (31)
#define PK_OP_POP (32)
#define PK_OP_POPN (33)

typedef enum PK_OPCODE_TY_ {
    PK_OPCODE_TY_NORMAL = 0,
    PK_OPCODE_TY_LOAD,
    PK_OPCODE_TY_LIT
} PK_OPCODE_TY;

typedef struct PKAtomLFun_ {
    PKAtomHeader header;
    PKArity arity;
    PKBlob bc;
    PKAtoms atoms;
    PKHashmap props;
} PKAtomLFun;

typedef struct PKAtomArray_ {
    PKAtomHeader header;
    PKArray atoms;
} PKAtomArray;

typedef struct PKAtomObject_ PKAtomObject;
struct PKAtomObject_ {
    PKAtomHeader header;
    pk_u8 reference;
    union {
        PKAtomSymbol *type;
        PKAtomObject *reference;
    } as;
    void *user_data;
    PKGCFun gc;
    PKHashmap props;
};

typedef struct PKAtomBlob_ {
    PKAtomHeader header;
    PKBlob bytes;
} PKAtomBlob;

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
    PKAtomArray array;
    PKAtomObject object;
    PKAtomBlob blob;
} PKAtomData;

typedef struct PKJmp_ PKJmp;
struct PKJmp_ {
    jmp_buf buf;
};

#define PK_FRAME_EMPTY (PKFrame){ \
    .stack_offset = 0, \
    .stack_length = 0, \
    .let_offset = 0, \
    .catch = NULL, \
    .jmp = NULL, \
    .mode = PK_FRAME_MODE_PROTECT, \
    .function = &PK_ATOM_NIL, \
}

#define PK_FRAME_MODE_NORMAL (0)
#define PK_FRAME_MODE_PROTECT (1)
#define PK_FRAME_MODE_CATCH (2)

typedef struct PKFrame_ {
    size_t stack_offset;
    size_t stack_length;
    size_t let_offset;
    PKAtomSymbol *catch;
    PKJmp *jmp;
    pk_u8 mode;
    PKAtom *function;
} PKFrame;

#define PK_FRAMES_EMPTY (PKFrames){.e = NULL, .count = 0, .capacity = 0}

typedef struct PKFrames {
    PKFrame *e;
    size_t count;
    size_t capacity;
} PKFrames;

typedef struct PKLet_ {
    PK_ENV env;
    PKAtomSymbol *symbol;
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
    .user = NULL, \
    .fastcall = NULL, \
    .lambda = NULL, \
    .macro = NULL, \
    .defvar = NULL, \
    .defun = NULL, \
    .defmacro = NULL, \
    .progn = NULL, \
    .sym_if = NULL, \
    .sym_while = NULL, \
    .error = NULL, \
    .throw = NULL, \
    .var = NULL, \
    .fvar = NULL, \
    .setq = NULL, \
    .fsetq = NULL, \
    .let = NULL, \
    .let_star = NULL, \
    .flet = NULL, \
    .flet_star = NULL, \
    .block = NULL, \
    .apply = NULL, \
    .macro_apply = NULL, \
    .quote = NULL, \
    .quasiquote = NULL, \
    .unquote = NULL, \
    .unquote_splice = NULL, \
    .string_substitute = NULL, \
}

typedef struct PKCache_ {
    PKAtomSymbol *user;
    PKAtomSymbol *fastcall;
    PKAtomSymbol *lambda;
    PKAtomSymbol *macro;
    PKAtomSymbol *defvar;
    PKAtomSymbol *defun;
    PKAtomSymbol *defmacro;
    PKAtomSymbol *progn;
    PKAtomSymbol *sym_if;
    PKAtomSymbol *sym_while;
    PKAtomSymbol *sym_and;
    PKAtomSymbol *sym_or;
    PKAtomSymbol *error;
    PKAtomSymbol *throw;
    PKAtomSymbol *var;
    PKAtomSymbol *fvar;
    PKAtomSymbol *setq;
    PKAtomSymbol *fsetq;
    PKAtomSymbol *let;
    PKAtomSymbol *let_star;
    PKAtomSymbol *protect;
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
    .keywordizer = PK_HASHMAP_EMPTY, \
    .cloner = PK_HASHMAP_EMPTY, \
    .writer = PK_WRITER_EMPTY, \
    .gensym_counter = 0, \
    .gc_enabled = false, \
    .current_frame = PK_FRAME_EMPTY, \
    .throw_symbol = &PK_ATOM_NIL, \
    .throw_value = &PK_ATOM_NIL, \
    .pool = NULL, \
    .free = NULL, \
    .cache = PK_CACHE_EMPTY, \
}

typedef struct PocketLispMachine_ {
    PKArray stack;
    PKLets lets;
    PKFrames frames;
    PKFrame current_frame;
    PKAtom *throw_symbol;
    PKAtom *throw_value;
    PKHashmap symbolizer;
    PKHashmap keywordizer;
    PKHashmap cloner;
    PKWriter writer;
    size_t gensym_counter;
    bool gc_enabled;
    PKPool *pool;
    PKAtomFree *free;
    PKCache cache;
} PocketLispMachine;

#define PK_ATOM_STRING_CACHE_LENGTH (128)
#define PK_ATOM_NUMBER_CACHE_LENGTH (101)

extern PKAtom PK_ATOM_NIL;
extern PKAtom PK_ATOM_TRUE;
extern PKAtom PK_ATOM_UNDEFINED;
extern PKAtomString PK_ATOM_EMPTY_STRING;
extern PKAtomString PK_ATOM_OUT_OF_MEMORY;
extern PKAtomBlob PK_ATOM_EMPTY_BLOB;
extern PKAtomString PK_ATOM_STRING_CACHE[PK_ATOM_STRING_CACHE_LENGTH];
extern PKAtomNumber PK_ATOM_NUMBER_CACHE[PK_ATOM_NUMBER_CACHE_LENGTH];

size_t pk_grow_capacity(size_t old_capacity, size_t init_capacity);
    
void *pk_malloc(Pocket lisp, size_t size);
void *pk_realloc(Pocket lisp, void *ptr, size_t new_size);
void pk_free(Pocket lisp, void *ptr);

void pk_pool_expand(Pocket lisp);

PKAtom *pk_atom_alloc(Pocket lisp);
void pk_atom_free(Pocket lisp, PKAtom *atom);

void pk_throw_clear(Pocket lisp);
void pk_error_atom(Pocket lisp, PKAtom *message);
void pk_error_type(Pocket lisp, PKAtom *found, PK_TYPE expected);
void pk_error_type2(Pocket lisp, PKAtom *found, PK_TYPE expected1, PK_TYPE expected2);
void pk_error_type3(Pocket lisp, PKAtom *found, PK_TYPE expected1, PK_TYPE expected2, PK_TYPE expected3);
void pk_error_type4(Pocket lisp, PKAtom *found, PK_TYPE expected1, PK_TYPE expected2, PK_TYPE expected3, PK_TYPE expected4);
void pk_error_writer(Pocket lisp);
PKAtom *pk_error_value(Pocket lisp);

bool pk_atom_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
bool pk_atom_eq_atoms(Pocket lisp, PKAtoms atoms);
bool pk_atom_struct_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
bool pk_atom_struct_eq_atoms(Pocket lisp, PKAtoms atoms);
bool pk_atom_object_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
bool pk_atom_object_eq_atoms(Pocket lisp, PKAtoms atoms);
bool pk_atom_text_qeq(Pocket lisp, PKAtom *text, PKStr cmp);
size_t pk_atom_hash(PKAtom *atom);

bool pk_atom_is_nil(PKAtom *atom);
bool pk_atom_is_true(PKAtom *atom);
bool pk_atom_is_undefined(PKAtom *atom);

PKAtomKind pk_atom_kind(PKAtom *atom);

PKAtom *pk_atom_clone(Pocket lisp, PKAtom *atom);

void pk_atom_assert_type(Pocket lisp, PKAtom *atom, PKAtomSymbol *type);

bool pk_atom_is_circular(Pocket lisp, PKAtom *atom);
bool pk_atom_is_cons(PKAtom *atom);
bool pk_atom_is_int(PKAtom *atom);
bool pk_atom_is_byte(PKAtom *atom);
bool pk_atom_is_float(PKAtom *atom);
bool pk_atom_is_number(PKAtom *atom);
bool pk_atom_is_string(PKAtom *atom);
bool pk_atom_is_blob(PKAtom *atom);
bool pk_atom_is_character(PKAtom *atom);
bool pk_atom_is_symbol(PKAtom *atom);
bool pk_atom_is_keyword(PKAtom *atom);
bool pk_atom_is_cfun(PKAtom *atom);
bool pk_atom_is_lfun(PKAtom *atom);
bool pk_atom_is_function(PKAtom *atom);
bool pk_atom_is_array(PKAtom *atom);
bool pk_atom_is_object(PKAtom *atom);
bool pk_atom_is_immortal(PKAtom *atom);
bool pk_atom_is_cmacro(PKAtom *atom);
bool pk_atom_is_macro(PKAtom *atom);
bool pk_atom_is_lmacro(PKAtom *atom);

bool pk_atom_is_macro(PKAtom *atom);

PKAtom *pk_atom_assert_nil(Pocket lisp, PKAtom *atom);
PKAtomCons *pk_atom_assert_cons(Pocket lisp, PKAtom *atom);
PKAtomNumber *pk_atom_assert_number(Pocket lisp, PKAtom *atom);
PKAtomString *pk_atom_assert_string(Pocket lisp, PKAtom *atom);
PKAtomString *pk_atom_assert_string_nil_is_empty(Pocket lisp, PKAtom *atom);
PKAtomSymbol *pk_atom_assert_symbol(Pocket lisp, PKAtom *atom);
PKAtomKeyword *pk_atom_assert_keyword(Pocket lisp, PKAtom *atom);
PKAtomLFun *pk_atom_assert_lfun(Pocket lisp, PKAtom *atom);
PKAtomLFun *pk_atom_assert_lfun_or_lmacro(Pocket lisp, PKAtom *atom);
PKAtomCFun *pk_atom_assert_cfun(Pocket lisp, PKAtom *atom);
PKAtomArray *pk_atom_assert_array(Pocket lisp, PKAtom *atom);
PKAtomObject *pk_atom_assert_object(Pocket lisp, PKAtom *atom);
PKAtomBlob *pk_atom_assert_blob(Pocket lisp, PKAtom *atom);

PKAtom *pk_atom_new_nil(Pocket lisp);
PKAtom *pk_atom_new_true(Pocket lisp);
PKAtom *pk_atom_new_undefined(Pocket lisp);
PKAtom *pk_atom_bool(Pocket lisp, bool boolean);

PKAtomString *pk_atom_string_new_empty(Pocket lisp);
PKAtomString *pk_atom_string_nomemcpy(Pocket lisp, PKStr string);
PKAtomString *pk_atom_string(Pocket lisp, PKStr string);
PKAtomString *pk_atom_string_char(Pocket lisp, char c);
PKAtomString *pk_atom_string_alloc(Pocket lisp, size_t length);
PKAtomString *pk_atom_stringc(Pocket lisp, char *cstring);
PKAtomString *pk_atom_string_cat(Pocket lisp, PKAtoms atoms);
PKAtomString *pk_atom_string_reverse(Pocket lisp, PKStr input);
PKAtomString *pk_atom_string_el(Pocket lisp, PKAtomString *string, size_t index);
PKAtom *pk_atom_string_split(Pocket lisp, PKAtomString *string, PKAtomString *pattern);
bool pk_atom_string_eq(Pocket lisp, PKAtomString *lhs, PKAtomString *rhs);
PKAtomString *pk_atom_string_make(Pocket lisp, size_t count, PKAtomString *fill);
bool pk_atom_string_is_numeric(PKAtomString *string);
bool pk_atom_string_is_alphabetic(PKAtomString *string);
bool pk_atom_string_is_whitespace(PKAtomString *string);
bool pk_atom_string_is_alphanumeric(PKAtomString *string);
bool pk_atom_string_is_graphic(PKAtomString *string);
bool pk_atom_string_is_upcase(PKAtomString *string);
bool pk_atom_string_is_downcase(PKAtomString *string);
PKAtomString *pk_atom_string_upcase(Pocket lisp, PKAtomString *string);
PKAtomString *pk_atom_string_downcase(Pocket lisp, PKAtomString *string);
PKAtomString *pk_atom_string_map(Pocket lisp, PKAtomString *string, PKAtom *function);
PKAtomString *pk_atom_string_filter(Pocket lisp, PKAtomString *string, PKAtom *function);

PKAtomBlob *pk_atom_blob_new_empty(Pocket lisp);
PKAtomBlob *pk_atom_blob_nomemcpy(Pocket lisp, PKBlob blob);
PKAtomBlob *pk_atom_blob_alloc(Pocket lisp, size_t length);
PKAtomBlob *pk_atom_blob(Pocket lisp, PKBlob blob);
PKAtomBlob *pk_atom_blob_cat(Pocket lisp, PKAtoms atoms);
PKAtom *pk_atom_blob_el(Pocket lisp, PKAtomBlob *blob, size_t index);
size_t pk_atom_blob_length(Pocket lisp, PKAtomBlob *blob);
PKAtomString *pk_atom_blob_to_string(Pocket lisp, PKAtomBlob *blob);
PKAtomBlob *pk_atom_blob_reverse(Pocket lisp, PKBlob input);
PKAtom *pk_atom_blob_split(Pocket lisp, PKAtomBlob *blob, PKAtom *pattern);
bool pk_atom_blob_eq(Pocket lisp, PKAtomBlob *lhs, PKAtomBlob *rhs);
PKAtomBlob *pk_atom_blob_make(Pocket lisp, size_t count, PKAtom *fill);
PKAtomBlob *pk_atom_blob_map(Pocket lisp, PKAtomBlob *blob, PKAtom *function);
PKAtomBlob *pk_atom_blob_filter(Pocket lisp, PKAtomBlob *blob, PKAtom *function);
PKStr pk_atom_as_string(Pocket lisp, PKAtom *atom);
size_t pk_atom_string_length(Pocket lisp, PKAtomString *string);

char pk_atom_str_to_char(Pocket lisp, PKStr string);
PKAtomSymbol *pk_atom_string_to_symbol(Pocket lisp, PKAtomString *string);
PKAtomKeyword *pk_atom_string_to_keyword(Pocket lisp, PKAtomString *string);
PKAtomBlob *pk_atom_string_to_bytes(Pocket lisp, PKAtomString *string);
bool pk_atom_string_cmp(PKAtomString *string, PKStr comp);

PKAtomCons *pk_atom_cons(Pocket lisp, PKAtom *car, PKAtom *cdr);
PKAtom *pk_atom_car(Pocket lisp, PKAtom *atom);
PKAtom *pk_atom_cdr(Pocket lisp, PKAtom *atom);
PKAtom *pk_atom_car_strict(Pocket lisp, PKAtom *atom);
PKAtom *pk_atom_cdr_strict(Pocket lisp, PKAtom *atom);
PKAtom *pk_atom_set_car(Pocket lisp, PKAtom *atom, PKAtom *new_car);
PKAtom *pk_atom_set_cdr(Pocket lisp, PKAtom *atom, PKAtom *new_cdr);
PKAtomCons *pk_atom_list1(Pocket lisp, PKAtom *first);
PKAtomCons *pk_atom_list2(Pocket lisp, PKAtom *first, PKAtom *second);
PKAtomCons *pk_atom_list3(Pocket lisp, PKAtom *first, PKAtom *second, PKAtom *third);
PKAtom *pk_atom_list(Pocket lisp, PKAtoms atoms);
PKAtom *pk_atom_list_make(Pocket lisp, size_t count, PKAtom *fill);
PKAtom *pk_atom_list_packed(Pocket lisp, PKAtoms atoms);
PKAtom *pk_atom_list_rev(Pocket lisp, PKAtoms atoms);
PKAtom *pk_atom_list_rev_packed(Pocket lisp, PKAtoms atoms);
void pk_atom_list_append(Pocket lisp, PKAtomCons *cons, PKAtom *atom);
PKAtom *pk_atom_list_reverse(Pocket lisp, PKAtom *list);
PKAtom *pk_atom_list_el(Pocket lisp, PKAtom *list, size_t index);
size_t pk_atom_list_length(Pocket lisp, PKAtom *list);
bool pk_atom_list_is_element(Pocket lisp, PKAtom *list, PKAtom *key);
PKAtom *pk_atom_list_split(Pocket lisp, PKAtom *list, PKAtom *pattern);
PKAtom *pk_atom_list_map(Pocket lisp, PKAtom *list, PKAtom *function);
PKAtom *pk_atom_list_filter(Pocket lisp, PKAtom *list, PKAtom *function);
PKAtom *pk_atom_list_fap(Pocket lisp, PKAtom *list, PKAtom *function);
PKAtom *pk_atom_list_reduce(Pocket lisp, PKAtom *list, PKAtom *function, PKAtom *init);
PKAtom *pk_atom_assoc(Pocket lisp, PKAtom *alist, PKAtom *key);
PKAtom *pk_atom_alist_to_plist(Pocket lisp, PKAtom *alist);
PKAtom *pk_atom_plist_to_alist(Pocket lisp, PKAtom *plist);
PKAtom *pk_atom_list_flatten(Pocket lisp, PKAtoms atoms);
PKAtom *pk_atom_list_flatten_atom(Pocket lisp, PKAtom *list);
PKAtom *pk_atom_list_cat(Pocket lisp, PKAtoms atoms);
void pk_atom_list_to_array(Pocket lisp, PKAtom *list, PKArray *array);

PKAtomNumber *pk_atom_int(Pocket lisp, int integer);
PKAtomNumber *pk_atom_float(Pocket lisp, float floater);
PKAtomNumber *pk_atom_num(Pocket lisp, PKNum num);
PKAtomNumber *pk_atom_byte(Pocket lisp, pk_u8 byte);
PKAtomNumber *pk_atom_size_t(Pocket lisp, size_t value);
PKBlob pk_atom_number_to_bytes(PKAtomNumber *number);
void pk_atom_number_to_bytes_trimmed_writer(Pocket lisp, PKAtomNumber *number);
PKAtomBlob *pk_atom_number_to_bytes_trimmed(Pocket lisp, PKAtomNumber *number);
PKAtomNumber *pk_atom_number_add(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
PKAtomNumber *pk_atom_number_sub(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
PKAtomNumber *pk_atom_number_mul(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
PKAtomNumber *pk_atom_number_div(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
PKAtomNumber *pk_atom_number_add_vec(Pocket lisp, PKAtoms vec);
PKAtomNumber *pk_atom_number_sub_vec(Pocket lisp, PKAtoms vec);
PKAtomNumber *pk_atom_number_mul_vec(Pocket lisp, PKAtoms vec);
PKAtomNumber *pk_atom_number_div_vec(Pocket lisp, PKAtoms vec);
PKAtomNumber *pk_atom_number_mod(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
bool pk_atom_number_eq(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
bool pk_atom_number_lt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
bool pk_atom_number_lte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
bool pk_atom_number_gt(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
bool pk_atom_number_gte(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
bool pk_atom_number_eq_vec(Pocket lisp, PKAtoms vec);
bool pk_atom_number_lt_vec(Pocket lisp, PKAtoms vec);
bool pk_atom_number_lte_vec(Pocket lisp, PKAtoms vec);
bool pk_atom_number_gt_vec(Pocket lisp, PKAtoms vec);
bool pk_atom_number_gte_vec(Pocket lisp, PKAtoms vec);

PKAtomNumber *pk_atom_number_sin(Pocket lisp, PKAtomNumber *number);
PKAtomNumber *pk_atom_number_cos(Pocket lisp, PKAtomNumber *number);
PKAtomNumber *pk_atom_number_tan(Pocket lisp, PKAtomNumber *number);
PKAtomNumber *pk_atom_number_exp(Pocket lisp, PKAtomNumber *number);
PKAtomNumber *pk_atom_number_log(Pocket lisp, PKAtomNumber *number);
PKAtomNumber *pk_atom_number_asin(Pocket lisp, PKAtomNumber *number);
PKAtomNumber *pk_atom_number_acos(Pocket lisp, PKAtomNumber *number);
PKAtomNumber *pk_atom_number_atan(Pocket lisp, PKAtomNumber *number);
PKAtomNumber *pk_atom_number_sqrt(Pocket lisp, PKAtomNumber *number);
PKAtomNumber *pk_atom_number_floor(Pocket lisp, PKAtomNumber *number);
PKAtomNumber *pk_atom_number_ceil(Pocket lisp, PKAtomNumber *number);
PKAtomNumber *pk_atom_number_truncate(Pocket lisp, PKAtomNumber *number);
PKAtomNumber *pk_atom_number_round(Pocket lisp, PKAtomNumber *number);
bool pk_atom_number_is_infinite(Pocket lisp, PKAtomNumber *number);
bool pk_atom_number_is_nan(Pocket lisp, PKAtomNumber *number);
bool pk_atom_number_is_zero(Pocket lisp, PKAtomNumber *number);
bool pk_atom_number_is_positive(Pocket lisp, PKAtomNumber *number);
bool pk_atom_number_is_negative(Pocket lisp, PKAtomNumber *number);
bool pk_atom_number_is_odd(Pocket lisp, PKAtomNumber *number);
bool pk_atom_number_is_even(Pocket lisp, PKAtomNumber *number);
PKAtomNumber *pk_atom_number_max_vec(Pocket lisp, PKAtoms vec);
PKAtomNumber *pk_atom_number_min_vec(Pocket lisp, PKAtoms vec);
PKAtomNumber *pk_atom_number_floor_div(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
PKAtomNumber *pk_atom_number_floor_remainder(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
PKAtomNumber *pk_atom_number_truncate_div(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
PKAtomNumber *pk_atom_number_truncate_remainder(Pocket lisp, PKAtomNumber *lhs, PKAtomNumber *rhs);
PKAtomNumber *pk_atom_number_lerp(Pocket lisp, PKAtomNumber *from, PKAtomNumber *to, PKAtomNumber *delta);

int pk_atom_number_to_int(Pocket lisp, PKAtomNumber *number, PK_NUMBER_CONV conv);
size_t pk_atom_number_to_size_t(Pocket lisp, PKAtomNumber *number);
size_t pk_atom_number_to_size_t_or(Pocket lisp, PKAtom *atom, size_t or);
float pk_atom_number_to_float(Pocket lisp, PKAtomNumber *number, PK_NUMBER_CONV conv);
int pk_atom_as_int(PKAtom *atom);
float pk_atom_as_float(PKAtom *atom);

PKNum pk_num_int(int integer);
PKNum pk_num_float(float floater);
int pk_num_to_int(PKNum num);
float pk_num_to_float(PKNum num);
PKNum pk_num_add(Pocket lisp, PKNum lhs, PKNum rhs);
PKNum pk_num_sub(Pocket lisp, PKNum lhs, PKNum rhs);
PKNum pk_num_mul(Pocket lisp, PKNum lhs, PKNum rhs);
PKNum pk_num_div(Pocket lisp, PKNum lhs, PKNum rhs);

PKAtomSymbol *pk_atom_gensym(Pocket lisp);
PKAtomSymbol *pk_atom_symbol(Pocket lisp, PKStr symbol);
PKAtomSymbol *pk_atom_symbolc(Pocket lisp, char *csymbol);
PKAtomKeyword *pk_atom_keyword(Pocket lisp, PKStr keyword);
PKAtomKeyword *pk_atom_keywordc(Pocket lisp, char *ckeyword);
bool pk_atom_keyword_eq(Pocket lisp, PKAtomKeyword *lhs, PKAtomKeyword *rhs);
bool pk_atom_symbol_cmp(PKAtomSymbol *symbol, PKStr comp);
bool pk_atom_keyword_cmp(PKAtomKeyword *keyword, PKStr comp);
PKAtomString *pk_atom_symbol_to_string(Pocket lisp, PKAtomSymbol *symbol);
PKAtomKeyword *pk_atom_symbol_to_keyword(Pocket lisp, PKAtomSymbol *symbol);
PKAtomString *pk_atom_keyword_to_string(Pocket lisp, PKAtomKeyword *keyword);
PKAtomSymbol *pk_atom_keyword_to_symbol(Pocket lisp, PKAtomKeyword *keyword);

PKAtom *pk_atom_symbol_put(Pocket lisp, PKAtomSymbol *symbol, PKAtom *key, PKAtom *value);
PKAtom *pk_atom_symbol_remove(Pocket lisp, PKAtomSymbol *symbol, PKAtom *key);
PKAtom *pk_atom_symbol_get(Pocket lisp, PKAtomSymbol *symbol, PKAtom *key);
bool pk_atom_symbol_is_property(Pocket lisp, PKAtomSymbol *symbol, PKAtom *key);
PKAtom *pk_atom_symbol_to_plist(Pocket lisp, PKAtomSymbol *symbol);

PKAtomCFun *pk_atom_cfun(Pocket lisp, void *user_closure, PKFun fun, PK_CALL call, PKFunArgs *args);

PKAtomArray *pk_atom_array(Pocket lisp, PKAtoms init);
PKAtomArray *pk_atom_array_make(Pocket lisp, size_t count, PKAtom *fill);
void pk_atom_array_append(Pocket lisp, PKAtomArray *array, PKAtom *atom);
void pk_atom_array_append_many(Pocket lisp, PKAtomArray *array, PKAtoms atoms);
PKAtom *pk_atom_array_el(Pocket lisp, PKAtomArray *array, size_t index);
bool pk_atom_array_is_element(Pocket lisp, PKAtomArray *array, PKAtom *element);
size_t pk_atom_array_length(Pocket lisp, PKAtomArray *array);
PKAtom *pk_atom_array_to_list(Pocket lisp, PKAtomArray *array);

PKAtom *pk_env_set(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol, PKAtom *value);
PKAtom *pk_env_let(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol, PKAtom *value);
PKAtom *pk_env_fetch(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol);
PKAtom *pk_env_fetch_loose(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol);
PKAtom *pk_env_fetch_strict(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol);
void pk_env_let_unwind(Pocket lisp, size_t n);
PKAtom *pk_env_alist(Pocket lisp, PK_ENV env);
PKAtom *pk_atom_list_types(Pocket lisp);
    
void pk_atom_cfun_exec(Pocket lisp, PKAtomCFun *cfun);
void pk_atom_lfun_exec(Pocket lisp, PKAtomLFun *lfun);
PKAtomString *pk_lfun_hexdump(Pocket lisp, PKAtomLFun *lfun);
PKAtomString *pk_atom_hexdump(Pocket lisp, PKAtom *atom);

PKAtom *pk_atom_cfun_put(Pocket lisp, PKAtomCFun *cfun, PKAtom *key, PKAtom *value);
PKAtom *pk_atom_cfun_remove(Pocket lisp, PKAtomCFun *cfun, PKAtom *key);
PKAtom *pk_atom_cfun_get(Pocket lisp, PKAtomCFun *cfun, PKAtom *key);
bool pk_atom_cfun_is_property(Pocket lisp, PKAtomCFun *cfun, PKAtom *key);
PKAtom *pk_atom_cfun_to_plist(Pocket lisp, PKAtomCFun *cfun);

PKAtom *pk_atom_lfun_put(Pocket lisp, PKAtomLFun *lfun, PKAtom *key, PKAtom *value);
PKAtom *pk_atom_lfun_remove(Pocket lisp, PKAtomLFun *lfun, PKAtom *key);
PKAtom *pk_atom_lfun_get(Pocket lisp, PKAtomLFun *lfun, PKAtom *key);
bool pk_atom_lfun_is_property(Pocket lisp, PKAtomLFun *lfun, PKAtom *key);
PKAtom *pk_atom_lfun_to_plist(Pocket lisp, PKAtomLFun *lfun);

PK_TYPE pk_atom_typeof(PKAtom *atom);
bool pk_atom_is_type(PKAtom *atom, PK_TYPE type);

PKAtomObject *pk_atom_object(Pocket lisp, PKAtomSymbol *type, void *user_data, PKGCFun gc);
PKAtomObject *pk_atom_object_cons(Pocket lisp, PKAtomSymbol *type, void *user_data, PKGCFun gc, PKAtoms init);
PKAtomObject *pk_atom_object_reference(Pocket lisp, PKAtomObject *original);
PKAtomObject *pk_atom_object_deref(PKAtomObject *object);
void *pk_atom_object_data(Pocket lisp, PKAtomObject *object, PKAtom *typecheck);
PKAtom *pk_atom_object_put(Pocket lisp, PKAtomObject *object, PKAtom *key, PKAtom *value);
PKAtom *pk_atom_object_remove(Pocket lisp, PKAtomObject *object, PKAtom *key);
PKAtom *pk_atom_object_get(Pocket lisp, PKAtomObject *object, PKAtom *key);
bool pk_atom_object_is_property(Pocket lisp, PKAtomObject *object, PKAtom *key);
size_t pk_atom_object_length(Pocket lisp, PKAtomObject *object);
PKAtom *pk_atom_object_to_plist(Pocket lisp, PKAtomObject *object);
PKAtom *pk_atom_object_to_alist(Pocket lisp, PKAtomObject *object);
    
PKAtom *pk_atom_list_symbols(Pocket lisp);

void pk_internal_print(Pocket lisp, PKStr print);
void pk_internal_puts(Pocket lisp, PKStr print);
void pk_atom_print(Pocket lisp, PKAtom *atom);
void pk_atom_puts(Pocket lisp, PKAtom *atom);
PKAtomString *pk_atom_format(Pocket lisp, PKAtom *atom);

void pk_frame_push(Pocket lisp, PKAtom *function, size_t arity);
void pk_frame_mode_set_protect(Pocket lisp);
void pk_frame_mode_set_catch(Pocket lisp, PKAtomSymbol *symbol);
void pk_frame_unwind(Pocket lisp);
void pk_frame_unwindn(Pocket lisp, size_t n);
void pk_frame_clear(Pocket lisp);
void pk_frame_return(Pocket lisp);
void pk_frame_return_push(Pocket lisp);
size_t pk_frame_length(Pocket lisp);

PKAtom *pk_generic_el(Pocket lisp, PKAtom *atom, size_t index);
bool pk_generic_is_element(Pocket lisp, PKAtom *atom, PKAtom *element);
size_t pk_generic_length(Pocket lisp, PKAtom *atom);
PKAtom *pk_generic_to_list(Pocket lisp, PKAtom *atom);
PKAtom *pk_generic_cat(Pocket lisp, PKAtom *atom, PKAtoms atoms);
PKAtom *pk_generic_split(Pocket lisp, PKAtom *atom, PKAtom *pattern);
PKAtom *pk_generic_reverse(Pocket lisp, PKAtom *atom);
PKAtomString *pk_generic_to_string(Pocket lisp, PKAtom *atom);
PKAtomBlob *pk_generic_to_bytes(Pocket lisp, PKAtom *atom);
PKAtomBlob *pk_generic_to_bytes_raw(Pocket lisp, PKAtom *atom);
PKAtomNumber *pk_generic_to_int(Pocket lisp, PKAtom *atom);
PKAtomNumber *pk_generic_to_float(Pocket lisp, PKAtom *atom);
PKAtomSymbol *pk_generic_to_symbol(Pocket lisp, PKAtom *atom);
PKAtomKeyword *pk_generic_to_keyword(Pocket lisp, PKAtom *atom);
PKAtom *pk_generic_property_put(Pocket lisp, PKAtom *pobj, PKAtom *key, PKAtom *value);
PKAtom *pk_generic_property_get(Pocket lisp, PKAtom *pobj, PKAtom *key);
PKAtom *pk_generic_property_remove(Pocket lisp, PKAtom *pobj, PKAtom *key);
bool pk_generic_property_is_property(Pocket lisp, PKAtom *pobj, PKAtom *key);
PKAtom *pk_generic_properties(Pocket lisp, PKAtom *pobj);

void pk_jmp_trigger(Pocket lisp, PKAtomSymbol *symbol, PKAtom *value);
void pk_jmp_leave(Pocket lisp);
void pk_jmp_enter(Pocket lisp, PKJmp *jmp);

void pk_stack_push(Pocket lisp, PKAtom *atom);
void pk_stack_push_many(Pocket lisp, PKAtoms atoms);
size_t pk_sp_index(Pocket lisp, int stack_pointer);
PKAtom *pk_stack_get(Pocket lisp, int stack_pointer);
PKAtom *pk_stack_set(Pocket lisp, int stack_pointer, PKAtom *value);
PKAtom *pk_stack_head(Pocket lisp);
PKAtom *pk_stack_head_pop(Pocket lisp);
PKAtom *pk_stack_head_strict(Pocket lisp);
void pk_stack_head_set_unchecked(Pocket lisp, PKAtom *atom);
PKAtom *pk_stack_pop(Pocket lisp);
void pk_stack_pop_unchecked(Pocket lisp, size_t n);
void pk_stack_set_unchecked(Pocket lisp, size_t count);
PKAtoms pk_stack_slice(Pocket lisp);
PKAtoms pk_stack_slice_down(Pocket lisp, size_t depth);

void pk_stack_op_dig(Pocket lisp, size_t index);
void pk_stack_op_car(Pocket lisp);
void pk_stack_op_cdr(Pocket lisp);
void pk_stack_op_car_strict(Pocket lisp);
void pk_stack_op_cdr_strict(Pocket lisp);
void pk_stack_op_list(Pocket lisp, size_t depth);
void pk_stack_op_list_rev(Pocket lisp, size_t depth);
void pk_stack_op_list_packed(Pocket lisp, size_t depth);
void pk_stack_op_list_rev_packed(Pocket lisp, size_t depth);
void pk_stack_op_list_merge(Pocket lisp, size_t depth);
void pk_stack_op_strcat(Pocket lisp, size_t depth);
    
PKAtom *pk_parse_atom(Pocket lisp, PKParser *r);
PKAtom *pk_parse_atom_barred(Pocket lisp, PKParser *r, PK_SYMTY type);
PKAtom *pk_parse_atom_keyword(Pocket lisp, PKParser *r);
PKAtom *pk_parse_atom_string(Pocket lisp, PKParser *r);
PKAtom *pk_parse_atom_number(Pocket lisp, PKParser *r);
PKNum pk_parse_num(Pocket lisp, PKParser *r);
PKAtom *pk_parse_atom_symbol(Pocket lisp, PKParser *r);
PKAtom *pk_parse_atom_cons(Pocket lisp, PKParser *r);
PKAtom *pk_parse_atom_blob(Pocket lisp, PKParser *r);

PKAtom *pk_parse_atom_from_str(Pocket lisp, PKStr string, PK_MODE mode);
PKAtom *pk_parse_atom_from_string(Pocket lisp, PKAtomString *string, PK_MODE mode);
    
void pk_writer_destroy(Pocket lisp);
void pk_writer_reset(Pocket lisp);
void pk_writer_char(Pocket lisp, char c);
void pk_writer_byte(Pocket lisp, pk_u8 byte);
void pk_writer_bytes(Pocket lisp, PKBlob bytes);
void pk_writer_cstr(Pocket lisp, const char *cstr);
void pk_writer_string(Pocket lisp, PKStr string);
void pk_writer_string_escaped(Pocket lisp, PKStr string);
void pk_writer_vfmt(Pocket lisp, const char *fmt, va_list args);
PK_PRINTF_FMT(2, 3) void pk_writer_fmt(Pocket lisp, const char *fmt, ...);
void pk_writer_int(Pocket lisp, int integer);
void pk_writer_address(Pocket lisp, size_t address);
void pk_writer_newline(Pocket lisp);
PKBlob pk_writer_slice(Pocket lisp);
PKStr pk_writer_str(Pocket lisp);
void pk_writer_symbol(Pocket lisp, PKStr ident);
void pk_writer_atom(Pocket lisp, PKAtom *atom);
    
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

pk_u8 pk_byte_cast(Pocket lisp, size_t word);
pk_u8 pk_byte_inc(Pocket lisp, pk_u8 byte);

PKBlob pk_blob_alloc(Pocket lisp, size_t length);
PKBlob pk_blob_dupe(Pocket lisp, PKBlob input);
bool pk_blob_eq(PKBlob a, PKBlob b);
void pk_blob_free(Pocket lisp, PKBlob *blob);
pk_u8 pk_blob_to_byte(Pocket lisp, PKBlob blob);
void pk_blob_reverse_mut(PKBlob blob);
    
PKAccum pk_accum_new(Pocket lisp);
PKAccum pk_accum_new_with(Pocket lisp, PKAtom *init);
void pk_accum_push(PKAccum *acc, PKAtom *atom);

void pk_array_push(Pocket lisp, PKArray *array, PKAtom *atom, size_t init_capacity);
void pk_array_deinit(Pocket lisp, PKArray *array);
PKAtoms pk_array_slice(PKArray *array);
PKAtoms pk_atoms_slice(PKAtoms atoms, size_t slice);
void pk_bytes_push(Pocket lisp, PKBytes *bytes, pk_u8 byte, size_t init_capacity);
void pk_bytes_deinit(Pocket lisp, PKBytes *bytes);
PKBlob pk_bytes_slice(PKBytes *bytes);
void pk_atoms_free(Pocket lisp, PKAtoms *atoms);
    
PKAtom *pk_hashmap_insert(Pocket lisp, PKHashmap *map, PKAtom *key, PKAtom *value, PK_HASHMAP_INSERT type);
PKAtom *pk_hashmap_get_str(Pocket lisp, PKHashmap *map, PKStr key);
PKAtom *pk_hashmap_get(Pocket lisp, PKHashmap *map, PKAtom *key);
PKAtom *pk_hashmap_remove(Pocket lisp, PKHashmap *map, PKAtom *key);
void pk_hashmap_deinit(Pocket lisp, PKHashmap *map);
PKAtom *pk_hashmap_to_alist(Pocket lisp, PKHashmap *map);
PKAtom *pk_hashmap_to_plist(Pocket lisp, PKHashmap *map);

PKAtom *pk_atom_property_put(Pocket lisp, PKAtom *object, PKHashmap *props, PKAtom *key, PKAtom *value);
PKAtom *pk_atom_property_remove(Pocket lisp, PKHashmap *props, PKAtom *key);
PKAtom *pk_atom_property_get(Pocket lisp, PKHashmap *props, PKAtom *key);
bool pk_atom_property_is_property(Pocket lisp, PKHashmap *props, PKAtom *key);
PKAtom *pk_atom_property_to_plist(Pocket lisp, PKHashmap *props);

size_t pk_hash_djb2(PKStr string);
size_t pk_hash_pointer(void *ptr);
bool pk_float_is_whole(float floater, int *integer);

void pk_compile_value(Pocket lisp, PKCompiler *c, PKAtom *value);
PKDSB pk_compile_destructure(Pocket lisp, PKCompiler *c, PKAtom *args);
size_t pk_compile_destructure_list(Pocket lisp, PKCompiler *c, PKAtom *list, size_t bias, size_t *pushed);
void pk_cmp_push_byte(Pocket lisp, PKCompiler *c, pk_u8 byte);
void pk_cmp_push_any(Pocket lisp, PKCompiler *c, size_t value);
void pk_cmp_push_atom(Pocket lisp, PKCompiler *c, PKAtom *atom);
void pk_cmp_load(Pocket lisp, PKCompiler *c, PKAtom *atom);
void pk_compile_strsub(Pocket lisp, PKCompiler *c, PKStr string);

PKStr pk_ident_atomty(pk_u8 type);
PKStr pk_ident_type(PK_TYPE type);
PKStr pk_ident_opcode(pk_u8 op);
PKStr pk_ident_arity(pk_u8 arity);
PKStr pk_ident_env(PK_ENV env);
PK_OPCODE_TY pk_opcode_ty(pk_u8 op);
    
PK_RES pk_int_funcall(Pocket lisp, PK_CALL call, size_t args);
PK_RES pk_int_fastcall(Pocket lisp, void *user_closure, PKFun fun, size_t args, PKAtom *frame);
PK_RES pk_protect_noreturn(Pocket lisp, void *user_closure, PKFun fun, size_t args);
PK_RES pk_atom_catch(Pocket lisp, PKAtomSymbol *symbol, void *user_closure, PKFun fun, size_t args);
PK_RES pk_atom_apply(Pocket lisp, PK_CALL call, PKAtom *fun, PKAtoms args);
PK_RES pk_atom_apply_atom(Pocket lisp, PK_CALL call, PKAtom *fun, PKAtom *args);
PK_RES pk_atom_eval(Pocket lisp, PKAtom *expr, PK_MODE mode);
bool pk_macroexpand1_(Pocket lisp, int atom);
bool pk_macroexpand_(Pocket lisp, int atom);
PKAtom *pk_atom_compile(Pocket lisp, PK_CALL type, PKAtom *value, PK_MODE mode);

void pk_std_load(Pocket lisp);

void pk_atom_deinit(Pocket lisp, PKAtom *atom);
PKAtom *pk_atom_destroy(Pocket lisp, PKAtom *atom);

PKVM pk_vm_init(PKAtom *lfun, size_t save);
void pk_vm_exec(Pocket lisp, PKVM *vm);

void pk_fs_file_write(Pocket lisp, PKAtomString *file_path, PKAtom *input);
void pk_fs_file_append(Pocket lisp, PKAtomString *file_path, PKAtom *input);
PKAtomString *pk_fs_file_read_string(Pocket lisp, PKAtomString *file_path);
PKAtomBlob *pk_fs_file_read_bytes(Pocket lisp, PKAtomString *file_path);
PKAtom *pk_fs_read_user_input(Pocket lisp);

void pk_atom_set_immortal(PKAtom *atom);
    
void pk_gc_mark_cache(Pocket lisp);
void pk_gc_mark(Pocket lisp, PKAtom *atom);
bool pk_gc_set_enable(Pocket lisp, bool enabled);
void pk_gc_collect(Pocket lisp);

#endif // POCKET_LISP_INTERNALS_H
