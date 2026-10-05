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

typedef struct PKAccum_ {
    PKAtom head;
    PKAtom tail;
} PKAccum;

#define PK_ATOMTY_ILLEGAL (0)
#define PK_ATOMTY_TYPE (1)
#define PK_ATOMTY_FREE (2)
#define PK_ATOMTY_ERROR (3)
#define PK_ATOMTY_NIL (4)
#define PK_ATOMTY_TRUE (5)
#define PK_ATOMTY_NUMBER (6)
#define PK_ATOMTY_STRING (7)
#define PK_ATOMTY_SYMBOL (8)
#define PK_ATOMTY_KEYWORD (9)
#define PK_ATOMTY_CONS (10)
#define PK_ATOMTY_ARRAY (11)
#define PK_ATOMTY_TABLE (12)
#define PK_ATOMTY_INTERPRETTER (13)

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

typedef struct PKAtomError_ {
    PKAtomHeader header;
} PKAtomError;

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
    PKAtom name;
} PKAtomSymbol;

typedef struct PKAtomCons_ {
    PKAtomHeader header;
    PKAtom car;
    PKAtom cdr;
} PKAtomCons;

typedef union PKAtomData_ {
    PKAtomHeader header;
    PKAtomFree free;
    PKAtomError error;
    PKAtomNumber number;
    PKAtomString string;
    PKAtomSymbol symbol;
    PKAtomCons cons;
} PKAtomData;

#define PK_POOL_MAX (1024)
typedef struct PKPool_ PKPool;

struct PKPool_ {
    PKAtomData e[PK_POOL_MAX];
    PKPool *next;
};

typedef struct PKCache_ {
    PKAtom quote;
    PKAtom quasiquote;
    PKAtom unquote;
    PKAtom unquote_splice;
    PKAtom string_substitute;
} PKCache;

typedef struct PocketLispMachine_ {
    PKPool *pool;
    PKAtomFree *free;
    PKCache cache;
    bool initialized;
} PocketLispMachine;

extern PocketLispMachine PK_CORE;

PKAtom pk_error(void);
PKAtom pk_error_oom(void);
    
size_t pk_grow_capacity(size_t old_capacity, size_t init_capacity);
    
void *pk_malloc(size_t size);
void *pk_realloc(void *ptr, size_t new_size);
void pk_free(void *ptr);

PKAtom pk_pool_expand(void);
PKAtom pk_atom_alloc(void);

PKAtom pk_read_atom(PKReader *r);
    
void pk_writer_deinit(PKWriter *w);
PKAtom pk_writer_expand(PKWriter *w, size_t needed);
PKAtom pk_writer_char(PKWriter *w, char c);
PKAtom pk_writer_string(PKWriter *w, PKStr string);
PKAtom pk_writer_string_escaped(PKWriter *w, PKStr string);
PK_PRINTF_FMT(2, 3) PKAtom pk_writer_fmt(PKWriter *w, const char *fmt, ...);
PKAtom pk_writer_int(PKWriter *w, int integer);
PKAtom pk_writer_float(PKWriter *w, float floater);
PKAtom pk_writer_address(PKWriter *w, size_t address);
PKAtom pk_writer_newline(PKWriter *w);
PKStr pk_writer_slice(PKWriter *w);
PKAtom pk_writer_atom(PKWriter *w, PKAtom atom);
    
PKAtom pk_str_dupe(PKStr input, PKStr *output);
bool pk_str_eq(PKStr a, PKStr b);
void pk_str_reverse_mut(char *c, size_t length);

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
size_t pk_hash_djb2(const char *c, size_t length);
    
#endif // POCKET_LISP_INTERNALS_H
