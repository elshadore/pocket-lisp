#ifndef POCKET_LISP_INTERNALS_H
#define POCKET_LISP_INTERNALS_H

#include "../include/pocket.h"
#include <setjmp.h>

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

#define PK_BLOB_EMPTY (PKBlob){.e = NULL, .length = 0}

typedef struct PKBlob_ {
    pk_u8 *e;
    size_t length;
} PKBlob;

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
#define PK_ATOMTY_CHARACTER (21)
#define PK_ATOMTY_BLOB (22)
#define PK_ATOMTY_STRING_ARRAY (23)
#define PK_ATOMTY_BYTE_ARRAY (24)
#define PK_ATOMTY_STREAM (25)
#define PK_ATOMTY_DESTROYED (26)

typedef enum PKAtomKind_ {
    PK_ATOMKIND_VALUE = 0,
    PK_ATOMKIND_OBJECT = 1,
} PKAtomKind;

#define PK_ATOMFLAG_NONE (0)
#define PK_ATOMFLAG_MARKED (1 << 0)

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
    PKAtomHeader header;
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
    PKAtomString *name;
    PKAtom *var;
    PKAtom *fun;
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
#define PK_OP_LET_VAR (15)
#define PK_OP_LET_FUN (16)
#define PK_OP_GET_VAR (17)
#define PK_OP_GET_FUN (18)
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
#define PK_OP_DEFCLASS (34)

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
} PKAtomLFun;

typedef struct PKAtomCObject_ {
    PKAtomHeader header;
    PKAtomSymbol *type;
    void *data;
} PKAtomCObject;

typedef struct PKAtomLObject_ {
    PKAtomHeader header;
    PKAtomSymbol *type;
    PKAtoms slots;
} PKAtomLObject;

typedef struct PKAtomChar_ {
    PKAtomHeader header;
    char c;
} PKAtomChar;

typedef struct PKAtomBlob_ {
    PKAtomHeader header;
    PKBlob bytes;
} PKAtomBlob;

typedef struct PKAtomArray_ {
    PKAtomHeader header;
    PKArray atoms;
} PKAtomArray;

typedef struct PKAtomTable_ {
    PKAtomHeader header;
    PKHashmap map;
} PKAtomTable;

typedef struct PKAtomStringArray_ {
    PKAtomHeader header;
    PKWriter string;
} PKAtomStringArray;

typedef struct PKAtomByteArray_ {
    PKAtomHeader header;
    PKBytes bytes;
} PKAtomByteArray;

typedef enum PK_READ_MODE_ {
    PK_READ_MODE_BINARY = 0,
    PK_READ_MODE_STRING,
} PK_READ_MODE;

typedef enum PK_OPEN_MODE_ {
    PK_OPEN_MODE_READ = 0,
    PK_OPEN_MODE_WRITE,
    PK_OPEN_MODE_APPEND,
    PK_OPEN_MODE_READ_WRITE,
    PK_OPEN_MODE_READ_APPEND,
} PK_OPEN_MODE;

typedef struct PKAtomStream_ {
    PKAtomHeader header;
    FILE *f;
    bool keep_open;
} PKAtomStream;

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
    PKAtomChar character;
    PKAtomBlob blob;
    PKAtomArray array;
    PKAtomTable table;
    PKAtomStringArray string_array;
    PKAtomByteArray byte_array;
    PKAtomStream stream;
} PKAtomData;

typedef struct PKJmp_ PKJmp;
struct PKJmp_ {
    jmp_buf buf;
};

#define PK_FRAME_EMPTY (PKFrame){ \
    .stack_offset = 0, \
    .let_offset = 0, \
    .catch = NULL, \
    .jmp = NULL, \
    .mode = PK_FRAME_MODE_PROTECT, \
    .function = NULL, \
}

#define PK_FRAME_MODE_NORMAL (0)
#define PK_FRAME_MODE_PROTECT (1)
#define PK_FRAME_MODE_CATCH (2)

typedef struct PKFrame_ {
    size_t stack_offset;
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
    .atom_true = NULL, \
    .nil = NULL, \
    .undefined = NULL, \
    \
    .empty_string = NULL, \
    .out_of_memory = NULL, \
    \
    .lambda = NULL, \
    .macro = NULL, \
    \
    .defvar = NULL, \
    .defun = NULL, \
    .defmacro = NULL, \
    \
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
    \
    .quote = NULL, \
    .quasiquote = NULL, \
    .unquote = NULL, \
    .unquote_splice = NULL, \
    .string_substitute = NULL, \
    \
    .type_unknown = NULL, \
    .type_undefined = NULL, \
    .type_nil = NULL, \
    .type_true = NULL, \
    .type_number = NULL, \
    .type_string = NULL, \
    .type_symbol = NULL, \
    .type_keyword = NULL, \
    .type_cons = NULL, \
    .type_array = NULL, \
    .type_table = NULL, \
    .type_function = NULL, \
    .type_macro = NULL, \
    .type_character = NULL, \
    .type_blob = NULL, \
    .type_string_array = NULL, \
    .type_byte_array = NULL, \
    .type_stream = NULL, \
    .type_destroyed = NULL, \
}

typedef struct PKCache_ {
    PKAtom *atom_true;
    PKAtom *nil;
    PKAtom *undefined;

    PKAtomString *empty_string;
    PKAtomString *out_of_memory;
    
    PKAtomSymbol *lambda;
    PKAtomSymbol *macro;
    
    PKAtomSymbol *defvar;
    PKAtomSymbol *defun;
    PKAtomSymbol *defmacro;
    PKAtomSymbol *defclass;
    
    PKAtomSymbol *progn;
    PKAtomSymbol *sym_if;
    PKAtomSymbol *sym_while;
    PKAtomSymbol *sym_and;
    PKAtomSymbol *sym_or;
    PKAtomSymbol *error;
    PKAtomSymbol *throw;
    PKAtomSymbol *cleared;
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

    PKAtomSymbol *type_unknown;
    PKAtomSymbol *type_undefined;
    PKAtomSymbol *type_nil;
    PKAtomSymbol *type_true;
    PKAtomSymbol *type_number;
    PKAtomSymbol *type_string;
    PKAtomSymbol *type_symbol;
    PKAtomSymbol *type_keyword;
    PKAtomSymbol *type_cons;
    PKAtomSymbol *type_array;
    PKAtomSymbol *type_table;
    PKAtomSymbol *type_function;
    PKAtomSymbol *type_macro;
    PKAtomSymbol *type_character;
    PKAtomSymbol *type_blob;
    PKAtomSymbol *type_string_array;
    PKAtomSymbol *type_byte_array;
    PKAtomSymbol *type_stream;
    PKAtomSymbol *type_destroyed;
} PKCache;

#define POCKET_LISP_MACHINE_EMPTY (PocketLispMachine) { \
    .stack = PK_ARRAY_EMPTY, \
    .lets = PK_LETS_EMPTY, \
    .frames = PK_FRAMES_EMPTY, \
    .symbolizer = PK_HASHMAP_EMPTY, \
    .gensym_counter = 0, \
    .gc_enabled = false, \
    .current_frame = PK_FRAME_EMPTY, \
    .throw_symbol = NULL, \
    .throw_value = NULL, \
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
    size_t gensym_counter;
    bool gc_enabled;
    PKPool *pool;
    PKAtomFree *free;
    PKCache cache;
} PocketLispMachine;

size_t pk_grow_capacity(size_t old_capacity, size_t init_capacity);
    
void *pk_malloc(Pocket lisp, size_t size);
void *pk_realloc(Pocket lisp, void *ptr, size_t new_size);
void pk_free(Pocket lisp, void *ptr);

void pk_pool_expand(Pocket lisp);

PKAtom *pk_atom_alloc(Pocket lisp);
void pk_atom_free(Pocket lisp, PKAtom *atom);

void pk_error_atom(Pocket lisp, PKAtom *message);
void pk_error_type(Pocket lisp, PKAtom *found, PKAtomSymbol *expected);
void pk_error_type2(Pocket lisp, PKAtom *found, PKAtomSymbol *expected1, PKAtomSymbol *expected2);
void pk_error_type3(Pocket lisp, PKAtom *found, PKAtomSymbol *expected1, PKAtomSymbol *expected2, PKAtomSymbol *expected3);
void pk_error_type4(Pocket lisp, PKAtom *found, PKAtomSymbol *expected1, PKAtomSymbol *expected2, PKAtomSymbol *expected3, PKAtomSymbol *expected4);
void pk_error_writer(Pocket lisp, PKWriter *w);
PKAtom *pk_error_value(Pocket lisp);

bool pk_atom_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
bool pk_atom_struct_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
bool pk_atom_object_eq(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
bool pk_atom_text_qeq(Pocket lisp, PKAtom *text, PKStr cmp);
size_t pk_atom_hash(PKAtom *atom);

bool pk_atom_is_nil(PKAtom *atom);
bool pk_atom_is_true(PKAtom *atom);
bool pk_atom_is_undefined(PKAtom *atom);

PKAtomKind pk_atom_kind(PKAtom *atom);

PKAtom *pk_atom_clone(Pocket lisp, PKAtom *atom);

bool pk_atom_is_type(Pocket lisp, PKAtom *atom, PKAtomSymbol *type);
bool pk_atom_is_type2(Pocket lisp, PKAtom *atom, PKAtomSymbol *type1, PKAtomSymbol *type2);
bool pk_atom_is_type3(Pocket lisp, PKAtom *atom, PKAtomSymbol *type1, PKAtomSymbol *type2, PKAtomSymbol *type3);
bool pk_atom_is_type4(Pocket lisp, PKAtom *atom, PKAtomSymbol *type1, PKAtomSymbol *type2, PKAtomSymbol *type3, PKAtomSymbol *type4);
bool pk_atom_is_typen(Pocket lisp, PKAtom *atom, PKAtoms types);

void pk_atom_assert_type(Pocket lisp, PKAtom *atom, PKAtomSymbol *type);

bool pk_atom_is_circular(Pocket lisp, PKAtom *atom);
bool pk_atom_is_cons(PKAtom *atom);
bool pk_atom_is_int(PKAtom *atom);
bool pk_atom_is_float(PKAtom *atom);
bool pk_atom_is_number(PKAtom *atom);
bool pk_atom_is_string(PKAtom *atom);
bool pk_atom_is_character(PKAtom *atom);
bool pk_atom_is_symbol(PKAtom *atom);
bool pk_atom_is_keyword(PKAtom *atom);
bool pk_atom_is_cfun(PKAtom *atom);
bool pk_atom_is_lfun(PKAtom *atom);
bool pk_atom_is_function(PKAtom *atom);
bool pk_atom_is_table(PKAtom *atom);
bool pk_atom_is_array(PKAtom *atom);
bool pk_atom_is_string_array(PKAtom *atom);
bool pk_atom_is_byte_array(PKAtom *atom);
bool pk_atom_is_object(PKAtom *atom);
bool pk_atom_is_cobject(PKAtom *atom);
bool pk_atom_is_lobject(PKAtom *atom);
bool pk_atom_is_blob(PKAtom *atom);
bool pk_atom_is_stream(PKAtom *atom);

PKAtom *pk_atom_reverse(Pocket lisp, PKAtom *atom);
bool pk_atom_is_macro(PKAtom *atom);

PKAtom *pk_atom_assert_nil(Pocket lisp, PKAtom *atom);
PKAtomCons *pk_atom_assert_cons(Pocket lisp, PKAtom *atom);
PKAtomNumber *pk_atom_assert_number(Pocket lisp, PKAtom *atom);
PKAtomString *pk_atom_assert_string(Pocket lisp, PKAtom *atom);
PKAtomChar *pk_atom_assert_character(Pocket lisp, PKAtom *atom);
PKAtomSymbol *pk_atom_assert_symbol(Pocket lisp, PKAtom *atom);
PKAtomKeyword *pk_atom_assert_keyword(Pocket lisp, PKAtom *atom);
PKAtomLFun *pk_atom_assert_lfun(Pocket lisp, PKAtom *atom);
PKAtomCFun *pk_atom_assert_cfun(Pocket lisp, PKAtom *atom);
PKAtomTable *pk_atom_assert_table(Pocket lisp, PKAtom *atom);
PKAtomCObject *pk_atom_assert_cobject(Pocket lisp, PKAtom *atom);
PKAtomLObject *pk_atom_assert_lobject(Pocket lisp, PKAtom *atom);
PKAtomArray *pk_atom_assert_array(Pocket lisp, PKAtom *atom);
PKAtomByteArray *pk_atom_assert_byte_array(Pocket lisp, PKAtom *atom);
PKAtomStringArray *pk_atom_assert_string_array(Pocket lisp, PKAtom *atom);
PKAtomBlob *pk_atom_assert_blob(Pocket lisp, PKAtom *atom);
PKAtomStream *pk_atom_assert_stream(Pocket lisp, PKAtom *atom);

PKAtom *pk_atom_new_nil(Pocket lisp);
PKAtom *pk_atom_new_true(Pocket lisp);
PKAtom *pk_atom_new_undefined(Pocket lisp);
PKAtom *pk_atom_bool(Pocket lisp, bool boolean);

PKAtomString *pk_atom_string_new_empty(Pocket lisp);
PKAtomString *pk_atom_string_nomemcpy(Pocket lisp, PKStr string);
PKAtomString *pk_atom_string(Pocket lisp, PKStr string);
PKAtomString *pk_atom_string_alloc(Pocket lisp, size_t length);
PKAtomString *pk_atom_stringc(Pocket lisp, char *cstring);
PKAtomString *pk_atom_string_cat(Pocket lisp, PKAtoms atoms);
char pk_atom_string_el(Pocket lisp, PKAtomString *string, size_t index);
PKStr pk_atom_as_string(Pocket lisp, PKAtom *atom);
size_t pk_atom_string_length(Pocket lisp, PKAtomString *string);

char pk_atom_str_to_char(Pocket lisp, PKStr string);
PKAtomChar *pk_atom_char(Pocket lisp, char c);
PKAtomChar *pk_atom_string_to_char(Pocket lisp, PKAtomString *string);
pk_u8 pk_atom_character_to_byte(Pocket lisp, PKAtomChar *character);
PKAtom *pk_atom_string_to_chars(Pocket lisp, PKAtomString *string);
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
PKAtom *pk_atom_list_packed(Pocket lisp, PKAtoms atoms);
PKAtom *pk_atom_list_rev(Pocket lisp, PKAtoms atoms);
PKAtom *pk_atom_list_rev_packed(Pocket lisp, PKAtoms atoms);
void pk_atom_list_append(Pocket lisp, PKAtomCons *cons, PKAtom *atom);
PKAtom *pk_atom_list_el(Pocket lisp, PKAtom *list, size_t index);
size_t pk_atom_list_length(Pocket lisp, PKAtom *list);
bool pk_atom_list_is_member(Pocket lisp, PKAtom *list, PKAtom *key);
PKAtom *pk_atom_list_map(Pocket lisp, PKAtom *list, PKAtom *function);
PKAtom *pk_atom_list_filter(Pocket lisp, PKAtom *list, PKAtom *function);
PKAtom *pk_atom_list_fap(Pocket lisp, PKAtom *list, PKAtom *function);
PKAtom *pk_atom_list_reduce(Pocket lisp, PKAtom *list, PKAtom *function, PKAtom *init);
PKAtom *pk_atom_assoc(Pocket lisp, PKAtom *alist, PKAtom *key);
PKAtom *pk_atom_to_list_bytes(Pocket lisp, PKBlob bytes);
PKAtom *pk_atom_to_list_chars(Pocket lisp, PKStr string);
PKAtom *pk_atom_to_list_table(Pocket lisp, PKHashmap *map);
PKAtom *pk_atom_to_list(Pocket lisp, PKAtom *atom);
PKAtom *pk_atom_list_flatten(Pocket lisp, PKAtoms atoms);
void pk_atom_list_to_array(Pocket lisp, PKAtom *list, PKArray *array);

PKAtomNumber *pk_atom_int(Pocket lisp, int integer);
PKAtomNumber *pk_atom_float(Pocket lisp, float floater);
PKAtomNumber *pk_atom_byte(Pocket lisp, pk_u8 byte);
PKAtomNumber *pk_atom_size_t(Pocket lisp, size_t value);
PKAtom *pk_atom_add(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
PKAtom *pk_atom_sub(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
PKAtom *pk_atom_mul(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
PKAtom *pk_atom_div(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
PKAtom *pk_atom_mod(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
bool pk_atom_lt(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
bool pk_atom_lte(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
bool pk_atom_gt(Pocket lisp, PKAtom *lhs, PKAtom *rhs);
bool pk_atom_gte(Pocket lisp, PKAtom *lhs, PKAtom *rhs);

int pk_atom_as_int(PKAtom *atom);
int pk_atom_to_int(Pocket lisp, PKAtom *atom, PK_NUMBER_CONV conv);
float pk_atom_as_float(PKAtom *atom);
float pk_atom_to_float(Pocket lisp, PKAtom *atom, PK_NUMBER_CONV conv);
size_t pk_atom_to_size_t(Pocket lisp, PKAtom *atom);
pk_u8 pk_atom_to_byte(Pocket lisp, PKAtom *atom);
pk_u8 pk_atom_number_to_byte(Pocket lisp, PKAtomNumber *number);
PKBlob pk_atom_as_bytes(Pocket lisp, PKAtom *atom);

PKAtomSymbol *pk_atom_gensym(Pocket lisp);
PKAtomSymbol *pk_atom_symbol(Pocket lisp, PKStr symbol);
PKAtomSymbol *pk_atom_symbolc(Pocket lisp, char *csymbol);
PKAtomKeyword *pk_atom_keyword(Pocket lisp, PKStr keyword);
PKAtomKeyword *pk_atom_keywordc(Pocket lisp, char *ckeyword);
bool pk_atom_symbol_cmp(PKAtomSymbol *symbol, PKStr comp);
bool pk_atom_keyword_cmp(PKAtomKeyword *keyword, PKStr comp);

PKAtomBlob *pk_atom_blob_new_empty(Pocket lisp);
PKAtomBlob *pk_atom_blob_nomemcpy(Pocket lisp, PKBlob blob);
PKAtomBlob *pk_atom_blob(Pocket lisp, PKBlob blob);
pk_u8 pk_atom_blob_el(Pocket lisp, PKAtomBlob *blob, size_t index);
size_t pk_atom_blob_length(Pocket lisp, PKAtomBlob *blob);

PKAtomCFun *pk_atom_cfun(Pocket lisp, void *user_closure, PKFun fun, PK_CALL call, PK_ARITY arity, pk_u8 args, pk_u8 options);

PKAtomArray *pk_atom_array(Pocket lisp, PKAtoms init);
PKAtomArray *pk_atom_array_make(Pocket lisp, size_t count, PKAtom *fill);
void pk_atom_array_append(Pocket lisp, PKAtomArray *array, PKAtom *atom);
void pk_atom_array_append_many(Pocket lisp, PKAtomArray *array, PKAtoms atoms);
PKAtom *pk_atom_array_el(Pocket lisp, PKAtomArray *array, size_t index);
size_t pk_atom_array_length(Pocket lisp, PKAtomArray *array);

PKAtomStringArray *pk_atom_string_array(Pocket lisp, PKAtoms init);
void pk_atom_string_array_append(Pocket lisp, PKAtomStringArray *array, PKAtom *atom);
void pk_atom_string_array_append_str(Pocket lisp, PKAtomStringArray *array, PKStr string);
char pk_atom_string_array_el(Pocket lisp, PKAtomStringArray *array, size_t index);
size_t pk_atom_string_array_length(Pocket lisp, PKAtomStringArray *array);

PKAtomByteArray *pk_atom_byte_array(Pocket lisp, PKAtoms init);
PKAtomByteArray *pk_atom_byte_array_cons(Pocket lisp, PKBlob init);
void pk_atom_byte_array_append(Pocket lisp, PKAtomByteArray *array, PKAtom *atom);
void pk_atom_byte_array_append_u8(Pocket lisp, PKAtomByteArray *array, pk_u8 byte);
void pk_atom_byte_array_append_blob(Pocket lisp, PKAtomByteArray *array, PKBlob blob);
pk_u8 pk_atom_byte_array_el(Pocket lisp, PKAtomByteArray *array, size_t index);
size_t pk_atom_byte_array_length(Pocket lisp, PKAtomByteArray *array);

PKAtom *pk_atom_array_to_list(Pocket lisp, PKAtomArray *array);
PKAtomString *pk_atom_string_array_to_string(Pocket lisp, PKAtomStringArray *array);
PKAtom *pk_atom_string_array_to_chars(Pocket lisp, PKAtomStringArray *array);
PKAtomChar *pk_atom_string_array_to_char(Pocket lisp, PKAtomStringArray *array);
pk_u8 pk_atom_byte_array_to_byte(Pocket lisp, PKAtomByteArray *array, size_t index);
PKBlob pk_atom_byte_array_to_bytes(Pocket lisp, PKAtomByteArray *array);
PKAtom *pk_atom_byte_array_to_list(Pocket lisp, PKAtomByteArray *array);

PKAtomTable * pk_atom_table(Pocket lisp);
PKAtomTable *pk_atom_table_cons(Pocket lisp, PKAtoms init);
PKAtom *pk_atom_table_put(Pocket lisp, PKAtomTable *table, PKAtom *key, PKAtom *value);
PKAtom *pk_atom_table_rem(Pocket lisp, PKAtomTable *table, PKAtom *key);
PKAtom *pk_atom_table_find(Pocket lisp, PKAtomTable *table, PKAtom *key);
bool pk_atom_table_is_member(Pocket lisp, PKAtomTable *table, PKAtom *key);
size_t pk_atom_table_length(Pocket lisp, PKAtomTable *table);
PKAtom *pk_atom_table_to_list(Pocket lisp, PKAtomTable *table);
    
void pk_generic_append(Pocket lisp, PKAtom *array, PKAtom *atom);
PKAtom *pk_generic_el(Pocket lisp, PKAtom *array, size_t index);
PKAtom *pk_generic_length_atom(Pocket lisp, PKAtom *seq);
size_t pk_generic_length(Pocket lisp, PKAtom *seq);
PKAtom *pk_generic_put(Pocket lisp, PKAtom *obj, PKAtom *key, PKAtom *value);
PKAtom *pk_generic_find(Pocket lisp, PKAtom *obj, PKAtom *key);
PKAtom *pk_generic_remove(Pocket lisp, PKAtom *obj, PKAtom *key);
PKAtom *pk_generic_is_member_atom(Pocket lisp, PKAtom *obj, PKAtom *key);
bool pk_generic_is_member(Pocket lisp, PKAtom *obj, PKAtom *key);
PKAtom *pk_generic_read_char(Pocket lisp, PKAtom *obj);
PKAtom *pk_generic_read_byte(Pocket lisp, PKAtom *obj);
PKAtom *pk_generic_read(Pocket lisp, PKAtom *obj, size_t amount, PKAtom *mode);
void pk_generic_write(Pocket lisp, PKAtom *obj, PKAtom *atom);

PKAtom *pk_generic_to_list(Pocket lisp, PKAtom *obj);
PKAtom *pk_generic_to_string(Pocket lisp, PKAtom *obj);
PKAtom *pk_generic_to_char(Pocket lisp, PKAtom *obj);
PKAtom *pk_generic_to_chars(Pocket lisp, PKAtom *obj);
PKAtom *pk_generic_to_bytes(Pocket lisp, PKAtom *obj);
PKAtom *pk_generic_to_byte(Pocket lisp, PKAtom *obj);
PKAtom *pk_generic_to_int(Pocket lisp, PKAtom *obj);
PKAtom *pk_generic_to_float(Pocket lisp, PKAtom *obj);
   
PKAtom *pk_env_get(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol);
PKAtom *pk_env_get_strict(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol);
PKAtom *pk_env_set(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol, PKAtom *value);
PKAtom *pk_env_let(Pocket lisp, PK_ENV env, PKAtomSymbol *symbol, PKAtom *value);
void pk_env_let_unwind(Pocket lisp, size_t n);
PKAtom *pk_env_alist(Pocket lisp, PK_ENV env);
PKAtom *pk_atom_list_types(Pocket lisp);
    
void pk_atom_cfun_exec(Pocket lisp, PKAtomCFun *cfun);
void pk_atom_lfun_exec(Pocket lisp, PKAtomLFun *lfun);
PKAtomString *pk_lfun_hexdump(Pocket lisp, PKAtomLFun *lfun);
PKAtomString *pk_atom_hexdump(Pocket lisp, PKAtom *atom);

void pk_atom_assert_untyped(Pocket lisp, PKAtomSymbol *type);
void pk_atom_define_type_lisp(Pocket lisp, PKAtomSymbol *type, PKAtoms slots);
void pk_atom_define_type_lisp_atom(Pocket lisp, PKAtomSymbol *type, PKAtom *slots);
void pk_atom_define_type_c_owned(Pocket lisp, PKAtomSymbol *type, size_t size);
void pk_atom_define_type_c_user(Pocket lisp, PKAtomSymbol *type);
void pk_atom_define_type_primitive(Pocket lisp, PKAtomSymbol *type);
PKAtomSymbol *pk_atom_typeof(Pocket lisp, PKAtom *atom);

PKAtom *pk_atom_object_new(Pocket lisp, PKAtomSymbol *type);
size_t pk_atom_slot_index(Pocket lisp, PKAtomLObject *object, PKAtomSymbol *slot);
PKAtom *pk_atom_slot_set(Pocket lisp, PKAtomLObject *object, PKAtomSymbol *slot, PKAtom *value);
PKAtom *pk_atom_slot_get(Pocket lisp, PKAtomLObject *object, PKAtomSymbol *slot);
void *pk_atom_cobject_data(Pocket lisp, PKAtomCObject *object);
size_t pk_atom_cobject_size(Pocket lisp, PKAtomCObject *object);

PKAtom *pk_atom_method_set(Pocket lisp, PKAtomSymbol *type, PKAtomSymbol *symbol, PKAtom *method);
PKAtom *pk_atom_list_methods(Pocket lisp, PKAtomSymbol *type);
PKAtom *pk_atom_list_implementors(Pocket lisp, PKAtom *method);
PKAtom *pk_atom_list_symbols(Pocket lisp);
PKAtom *pk_atom_method_get(Pocket lisp, PKAtomSymbol *type, PKAtomSymbol *symbol);
PKAtom *pk_atom_method_get_raw(Pocket lisp, PKAtomSymbol *type, PKAtomSymbol *symbol);

PKAtom *pk_atom_list_slots_type(Pocket lisp, PKAtomSymbol *type);
PKAtom *pk_atom_list_slots_object(Pocket lisp, PKAtomLObject *object);
PKAtom *pk_atom_list_slots(Pocket lisp, PKAtom *atom);

void pk_internal_println(Pocket lisp, PKStr print);
void pk_atom_print(Pocket lisp, PKAtom *atom);
void pk_atom_write(Pocket lisp, PKAtom *atom);
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

void pk_jmp_trigger(Pocket lisp, PKAtomSymbol *symbol, PKAtom *value);
void pk_jmp_leave(Pocket lisp);
void pk_jmp_enter(Pocket lisp, PKJmp *jmp);

void pk_stack_push(Pocket lisp, PKAtom *atom);
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
PKAtom *pk_parse_atom_barred(Pocket lisp, PKParser *r, PKWriter *w, PK_SYMTY type);
PKAtom *pk_parse_atom_keyword(Pocket lisp, PKParser *r);
PKAtom *pk_parse_atom_string(Pocket lisp, PKParser *r);
PKAtom *pk_parse_atom_number(Pocket lisp, PKParser *r);
PKAtom *pk_parse_atom_symbol(Pocket lisp, PKParser *r);
PKAtom *pk_parse_atom_cons(Pocket lisp, PKParser *r);

PKAtom *pk_parse_atom_from_str(Pocket lisp, PKStr string, PK_MODE mode);
PKAtom *pk_parse_atom_from_string(Pocket lisp, PKAtomString *string, PK_MODE mode);
    
void pk_writer_deinit(Pocket lisp, PKWriter *w);
void pk_writer_expand(Pocket lisp, PKWriter *w, size_t needed);
void pk_writer_char(Pocket lisp, PKWriter *w, char c);
void pk_writer_cstr(Pocket lisp, PKWriter *w, const char *cstr);
void pk_writer_string(Pocket lisp, PKWriter *w, PKStr string);
void pk_writer_string_escaped(Pocket lisp, PKWriter *w, PKStr string);
void pk_writer_vfmt(Pocket lisp, PKWriter *w, const char *fmt, va_list args);
PK_PRINTF_FMT(3, 4) void pk_writer_fmt(Pocket lisp, PKWriter *w, const char *fmt, ...);
void pk_writer_int(Pocket lisp, PKWriter *w, int integer);
void pk_writer_address(Pocket lisp, PKWriter *w, size_t address);
void pk_writer_newline(Pocket lisp, PKWriter *w);
PKStr pk_writer_slice(PKWriter *w);
void pk_writer_reset(PKWriter *w);
void pk_writer_symbol_name(Pocket lisp, PKWriter *w, PKStr name);
void pk_writer_atom(Pocket lisp, PKWriter *w, PKAtom *atom);
    
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
bool pk_char_from_cname(PKStr input, char *output);
bool pk_char_to_cname(char c, PKStr *output);
char pk_char_from_hex(pk_u8 byte);
bool pk_char_is_hex(char c);
pk_u8 pk_char_to_hex(char c);

PKBlob pk_blob_alloc(Pocket lisp, size_t length);
PKBlob pk_blob_dupe(Pocket lisp, PKBlob input);
void pk_blob_free(Pocket lisp, PKBlob *blob);
    
PKAccum pk_accum_new(Pocket lisp);
PKAccum pk_accum_new_with(Pocket lisp, PKAtom *init);
void pk_accum_push(PKAccum *acc, PKAtom *atom);

void pk_array_push(Pocket lisp, PKArray *array, PKAtom *atom, size_t init_capacity);
void pk_array_deinit(Pocket lisp, PKArray *array);
PKAtoms pk_array_slice(PKArray *array);
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

size_t pk_slotmap_hash(PKSlotmap *map, PKAtom *symbol);
PKSlotmapSlot *pk_slotmap_get(Pocket lisp, PKSlotmap *map, PKAtom *symbol);
void pk_slotmap_insert(Pocket lisp, PKSlotmap *map, PKAtom *symbol);
void pk_slotmap_deinit(Pocket lisp, PKSlotmap *map);
    
size_t pk_hash_djb2(PKStr string);
size_t pk_hash_pointer(void *ptr);

void pk_compile_value(Pocket lisp, PKCompiler *c, PKAtom *value);
void pk_cmp_push_byte(Pocket lisp, PKCompiler *c, pk_u8 byte);
void pk_cmp_push_any(Pocket lisp, PKCompiler *c, size_t value);
void pk_cmp_push_atom(Pocket lisp, PKCompiler *c, PKAtom *atom);
void pk_cmp_load(Pocket lisp, PKCompiler *c, PKAtom *atom);
void pk_compile_strsub(Pocket lisp, PKCompiler *c, PKStr string);

PKStr pk_ident_atomty(pk_u8 type);
PKStr pk_ident_opcode(pk_u8 op);
PKStr pk_ident_arity(pk_u8 arity);
PKStr pk_ident_env(PK_ENV env);
PK_OPCODE_TY pk_opcode_ty(pk_u8 op);
    
PK_RES pk_int_funcall(Pocket lisp, PK_CALL call, size_t args);
PK_RES pk_int_fastcall(Pocket lisp, void *user_closure, PKFun fun, size_t args, PKAtom *frame);

PK_RES pk_atom_catch(Pocket lisp, PKAtomSymbol *symbol, void *user_closure, PKFun fun, size_t args);
PK_RES pk_atom_apply(Pocket lisp, PK_CALL call, PKAtom *fun, PKAtom *args);
PK_RES pk_atom_eval(Pocket lisp, PKAtom *expr, PK_MODE mode);
bool pk_macroexpand1_(Pocket lisp, int atom);
bool pk_macroexpand_(Pocket lisp, int atom);
PKAtom *pk_atom_compile(Pocket lisp, PK_CALL type, PKAtom *value, PK_MODE mode);

void pk_std_load(Pocket lisp);

PKAtomString *pk_fs_slurp_str(Pocket lisp, PKStr file_path);
PKAtomString *pk_fs_slurp(Pocket lisp, PKAtomString *file_path);
PKAtom *pk_fs_read_user_input(Pocket lisp);

PKStr pk_atom_stream_mode_name(Pocket lisp, PKAtom *atom);
const char *pk_open_mode_to_str(PK_OPEN_MODE mode);
PK_OPEN_MODE pk_open_mode_from_str(Pocket lisp, PKStr name);

void pk_atom_deinit(Pocket lisp, PKAtom *atom);
PKAtom *pk_atom_destroy(Pocket lisp, PKAtom *atom);

PKAtomStream *pk_atom_stream_file_open(Pocket lisp, PKStr file_path, PK_OPEN_MODE mode);
PKAtomStream *pk_atom_stream_inherit(Pocket lisp, FILE *file, bool keep_open);
PKAtom *pk_atom_stream_read_char(Pocket lisp, PKAtomStream *stream);
PKAtom *pk_atom_stream_read_byte(Pocket lisp, PKAtomStream *stream);
PKAtom *pk_atom_stream_read(Pocket lisp, PKAtomStream *stream, size_t amount, PK_READ_MODE mode);
PKAtom *pk_atom_stream_read_atom(Pocket lisp, PKAtomStream *stream, size_t amount, PKAtom *mode);
void pk_atom_stream_write_string(Pocket lisp, PKAtomStream *stream, PKStr string);
void pk_atom_stream_write_bytes(Pocket lisp, PKAtomStream *stream, PKBlob blob);
void pk_atom_stream_write(Pocket lisp, PKAtomStream *stream, PKAtom *atom);
void pk_atom_stream_flush(Pocket lisp, PKAtomStream *stream);

PKVM pk_vm_init(PKAtom *lfun, size_t save);
void pk_vm_exec(Pocket lisp, PKVM *vm);

void pk_gc_mark_cache(Pocket lisp);
void pk_gc_mark(Pocket lisp, PKAtom *atom);
bool pk_gc_set_enable(Pocket lisp, bool enabled);
void pk_gc_collect(Pocket lisp);
void pk_gc_collect_no_cache(Pocket lisp);

#endif // POCKET_LISP_INTERNALS_H
