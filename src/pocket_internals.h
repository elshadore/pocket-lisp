#ifndef POCKET_LISP_INTERNALS_H
#define POCKET_LISP_INTERNALS_H

#include "pocket.h"

typedef int8_t pk_i8;
typedef int16_t pk_i16;
typedef int32_t pk_i32;
typedef int64_t pk_i64;

typedef uint8_t pk_u8;
typedef uint16_t pk_u16;
typedef uint32_t pk_u32;
typedef uint64_t pk_u64;

typedef enum PK_SYMTY_ {
    PK_SYMTY_SYMBOL = 0,
    PK_SYMTY_KEYWORD,
} PK_SYMTY;

#define PK_ARRAY_EMPTY (PKArray){.e = NULL, .count = 0, .capacity = 0}

#define PK_HASHMAP_EMPTY (PKHashmap){.e = NULL, .count = 0, .capacity = 0}

typedef struct PKArray_ {
    PKAtom *e;
    size_t count;
    size_t capacity;
} PKArray;

#define PK_BYTES_EMPTY (PKBytes){.e = NULL, .count = 0, .capacity = 0}

typedef struct PKBytes_ {
    pk_u8 *e;
    size_t count;
    size_t capacity;
} PKBytes;

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
} PKCompiler;

typedef struct PKVM_ {
    PKAtom lfun;
    size_t curr;
    size_t save;
    pk_u8 op;
    bool finished;
} PKVM;

typedef struct PKAccum_ {
    PKAtom head;
    PKAtom tail;
} PKAccum;

#define PK_HASHMAP_SLOT_UNOCCUPIED (0)
#define PK_HASHMAP_SLOT_OCCUPIED (1)
#define PK_HASHMAP_SLOT_DELETED (2)

typedef struct PKHashmapSlot_ {
    PKAtom key;
    PKAtom value;
    pk_u8 state;
} PKHashmapSlot;

typedef struct PKHashmap_ {
    PKHashmapSlot *e;
    size_t count;
    size_t capacity;
} PKHashmap;

#define PK_CALLCONV_EMPTY (PKCallConv){.args = 0, .variadic_count = 0, .variadic = false, .push_extra_nil = false}

typedef struct PKCallConv_ {
    pk_u8 args;
    pk_u8 variadic_count;
    bool variadic;
    bool push_extra_nil;
} PKCallConv;

#define PK_ATOMTY_ILLEGAL (0)
#define PK_ATOMTY_TYPE (1)
#define PK_ATOMTY_FREE (2)
#define PK_ATOMTY_OOM (3)
#define PK_ATOMTY_THROW (4)
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

#define PK_ATOMFLAG_NONE (0)
#define PK_ATOMFLAG_IMMORTAL (1 << 0)

typedef struct PKAtomHeader_ {
    pk_u8 type;
    pk_u8 flags;
    pk_u8 refcount;
} PKAtomHeader;

typedef struct PKAtomFree_ PKAtomFree;
struct PKAtomFree_ {
    PKAtomHeader header;
    PKAtomFree *next;
};

typedef struct PKAtomThrow_ {
    PKAtomHeader header;
    PKAtom type;
    PKAtom contents;
} PKAtomThrow;

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

typedef struct PKAtomSymbol_ {
    PKAtomHeader header;
    PKAtom var;
    PKAtom fun;
    PKAtom prop;
    PKAtom name;
} PKAtomSymbol;

typedef struct PKAtomKeyword_ {
    PKAtomHeader header;
    PKStr id;
} PKAtomKeyword;

typedef struct PKAtomCons_ {
    PKAtomHeader header;
    PKAtom car;
    PKAtom cdr;
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
    void *user_closure;
    union {
        void *opaque;
        PKFun0 *fun0;
        PKFun1 *fun1;
        PKFun2 *fun2;
        PKFun3 *fun3;
    } as;
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

typedef enum PK_OPCODE_TY_ {
    PK_OPCODE_TY_NORMAL = 0,
    PK_OPCODE_TY_LOAD,
    PK_OPCODE_TY_LIT
} PK_OPCODE_TY;

typedef struct PKAtomLFun_ {
    PKAtomHeader header;
    pk_u8 args;
    pk_u8 arity;
    struct {
        pk_u8 *e;
        size_t length;
    } bc;
    PKAtoms atoms;
} PKAtomLFun;

typedef union PKAtomData_ {
    PKAtomHeader header;
    PKAtomFree free;
    PKAtomThrow throw;
    PKAtomNumber number;
    PKAtomString string;
    PKAtomSymbol symbol;
    PKAtomKeyword keyword;
    PKAtomCons cons;
    PKAtomCFun cfun;
    PKAtomLFun lfun;
    PKAtomCFun cmacro;
    PKAtomLFun lmacro;
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
    PKAtom symbol;
    PKAtom restore;
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
    PKAtom lambda;
    PKAtom macro;
    
    PKAtom progn;
    PKAtom sym_if;
    PKAtom sym_while;
    PKAtom sym_and;
    PKAtom sym_or;
    PKAtom error;
    PKAtom throw;
    PKAtom var;
    PKAtom fvar;
    PKAtom let;
    PKAtom let_star;
    PKAtom flet;
    PKAtom flet_star;
    PKAtom block;
    PKAtom apply;
    PKAtom macro_apply;
    
    PKAtom quote;
    PKAtom quasiquote;
    PKAtom unquote;
    PKAtom unquote_splice;
    PKAtom string_substitute;
    
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

extern PocketLispMachine PK_CORE;

#define pk_frame_length() \
    ((PK_CORE.stack.count) - (PK_CORE.current_frame.stack_offset))

#define pk_writer_str(w_, string_) \
    pk_writer_string(w_, pk_str(string_))

size_t pk_grow_capacity(size_t old_capacity, size_t init_capacity);
    
void *pk_malloc(size_t size);
void *pk_realloc(void *ptr, size_t new_size);
void pk_free(void *ptr);

PKAtom pk_pool_expand(void);
PKAtom pk_atom_alloc(void);
void pk_atom_free(PKAtom atom);

PKAtom pk_string_empty(void);
PKAtom pk_string_alloc(size_t length);
PKAtom pk_string_nomemcpy(PKStr string);
PKAtom pk_string_cat(PKAtoms strings);

bool pk_type_match(PKAtom atom, PK_TYPE match);
    
PKAtom pk_undefined(void);

PKAtom pk_lfun_exec(PKAtom lfun, size_t save);    
bool pk_is_cfun(PKAtom atom);
bool pk_is_lfun(PKAtom atom);
bool pk_is_function(PKAtom atom);
PKAtom pk_typecheck_lfun(PKAtom atom);
PKAtom pk_typecheck_cfun(PKAtom atom);
PKAtom pk_typecheck_function(PKAtom atom);

void pk_print_str(PKStr string);

PKAtom pk_frame_push(size_t arity);
PKAtom pk_frame_pushc(PKAtoms atoms);
void pk_frame_clear(void);
void pk_frame_unwind(void);

PKAtom pk_stack_push(PKAtom atom);
PKAtom pk_stack_funcall(PK_CALL call, pk_u8 arity);
PKAtom pk_stack_head();
PKAtom pk_stack_pop();
void pk_stack_pop_unchecked(size_t n);
PKAtoms pk_stack_slice();

PKAtom pk_stack_slice_down(size_t depth, PKAtoms *output);
PKAtom pk_stack_op_list(size_t depth);
PKAtom pk_stack_op_list_packed(size_t depth);
PKAtom pk_stack_op_list_merge(size_t depth);
PKAtom pk_stack_op_strcat(size_t depth);
    
void pk_let_unwind(size_t n);

PKAtom pk_read_atom(PKReader *r);
PKAtom pk_read_barred(PKReader *r, PKWriter *w, PK_SYMTY type);
    
void pk_writer_deinit(PKWriter *w);
PKAtom pk_writer_expand(PKWriter *w, size_t needed);
PKAtom pk_writer_char(PKWriter *w, char c);
PKAtom pk_writer_cstr(PKWriter *w, const char *cstr);
PKAtom pk_writer_string(PKWriter *w, PKStr string);
PKAtom pk_writer_string_escaped(PKWriter *w, PKStr string);
PK_PRINTF_FMT(2, 3) PKAtom pk_writer_fmt(PKWriter *w, const char *fmt, ...);
PKAtom pk_writer_int(PKWriter *w, int integer);
PKAtom pk_writer_float(PKWriter *w, float floater);
PKAtom pk_writer_address(PKWriter *w, size_t address);
PKAtom pk_writer_newline(PKWriter *w);
PKStr pk_writer_slice(PKWriter *w);
void pk_writer_reset(PKWriter *w);
PKAtom pk_writer_symbol_name(PKWriter *w, PKStr name);
PKAtom pk_writer_atom(PKWriter *w, PKAtom atom);
    
PKAtom pk_str_alloc(size_t length, PKStr *output);
PKAtom pk_str_dupe(PKStr input, PKStr *output);
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

PKAccum pk_accum_new(void);
PKAtom pk_accum_push(PKAccum *acc, PKAtom atom);
void pk_cons_link(PKAtom tail, PKAtom cons);

PKAtom pk_array_push(PKArray *array, PKAtom atom, size_t init_capacity);
void pk_array_deinit(PKArray *array);
PKAtom pk_bytes_push(PKBytes *bytes, pk_u8 byte, size_t init_capacity);
    
PKAtom pk_hashmap_insert(PKHashmap *map, PKAtom key, PKAtom value);
PKAtom pk_hashmap_get_str(PKHashmap *map, PKStr key);
PKAtom pk_hashmap_get(PKHashmap *map, PKAtom key);
PKAtom pk_hashmap_remove(PKHashmap *map, PKAtom key);
void pk_hashmap_deinit(PKHashmap *map);
    
size_t pk_hash_djb2(const char *c, size_t length);
size_t pk_hash_pointer(void *ptr);
size_t pk_hash(PKAtom atom);

PKAtom pk_cmp_patch_byte(PKCompiler *c, pk_u8 addr, pk_u8 byte);
PKAtom pk_cmp_push_byte(PKCompiler *c, pk_u8 byte);
PKAtom pk_cmp_push_any(PKCompiler *c, size_t value);
PKAtom pk_cmp_push_atom(PKCompiler *c, PKAtom atom);
PKAtom pk_cmp_load_nil(PKCompiler *c);
PKAtom pk_cmp_load_true(PKCompiler *c);
PKAtom pk_cmp_load(PKCompiler *c, PKAtom atom);
    
PKAtom pk_compile_value(PKCompiler *c, PKAtom value);
PKAtom pk_compile_lambda(PK_CALL type, PKAtom args, PKAtom body);
PKAtom pk_compile_atom(PKCompiler *c, PKAtom value);
PKAtom pk_compile_strsub(PKCompiler *c, PKStr string);

PKStr pk_ident_opcode(pk_u8 op);
PKStr pk_ident_atomty(pk_u8 ty);
PKStr pk_ident_arity(pk_u8 arity);
PKStr pk_ident_env(PK_ENV env);
PKStr pk_ident_type(PK_TYPE ty);
PK_OPCODE_TY pk_opcode_ty(pk_u8 op);
    
PKAtom pk_macroexpand1_(PKAtom atom, bool *expanded);

extern PKAtomData PK_ATOM_OOM;
extern PKAtomData PK_ATOM_OOM_MESSAGE;

PKAtom pk_std_load(void);
void pk_deinit_cache(void);

PKAtom pk_fs_slurp_str(PKStr file_path);
PKAtom pk_fs_slurp(PKAtom file_path);
PKAtom pk_fs_read_user_input(void);

PKVM pk_vm_init(PKAtom lfun, size_t save);
PKAtom pk_vm_exec(PKVM *vm);

#ifdef POCKET_CFLAG_DEBUG
void pk_debug_leakcheck(void);
PKAtom pk_debug_dump(const char *tag, PK_DUMP opt);
#endif
    
#endif // POCKET_LISP_INTERNALS_H
