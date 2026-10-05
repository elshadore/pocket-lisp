#ifndef ULISP_INTERNALS_H
#define ULISP_INTERNALS_H

#include "user.h"

typedef int8_t ul_i8;
typedef int16_t ul_i16;
typedef int32_t ul_i32;
typedef int64_t ul_i64;

typedef uint8_t ul_u8;
typedef uint16_t ul_u16;
typedef uint32_t ul_u32;
typedef uint64_t ul_u64;

#define UL_WRITER_EMPTY (ULWriter){.c = NULL, .count = 0, .capacity = 0}

typedef struct ULWriter_ {
    char *c;
    size_t count;
    size_t capacity;
} ULWriter;

#define UL_ATOMTY_ILLEGAL (0)
#define UL_ATOMTY_TYPE (1)
#define UL_ATOMTY_FREE (2)
#define UL_ATOMTY_ERROR (3)
#define UL_ATOMTY_NIL (4)
#define UL_ATOMTY_TRUE (5)
#define UL_ATOMTY_NUMBER (6)
#define UL_ATOMTY_STRING (7)
#define UL_ATOMTY_SYMBOL (8)
#define UL_ATOMTY_KEYWORD (9)
#define UL_ATOMTY_CONS (10)
#define UL_ATOMTY_ARRAY (11)
#define UL_ATOMTY_TABLE (12)
#define UL_ATOMTY_INTERPRETTER (13)

typedef struct ULAtomHeader_ {
    ul_u8 type;
    size_t refcount;
} ULAtomHeader;

typedef struct ULAtomFree_ ULAtomFree;
struct ULAtomFree_ {
    ULAtomHeader header;
    ULAtomFree *next;
};

typedef struct ULAtomError_ {
    ULAtomHeader header;
} ULAtomError;

#define UL_NUMTY_INT (0)
#define UL_NUMTY_FLOAT (1)

typedef struct ULAtomNumber_ {
    ULAtomHeader header;
    ul_u8 num_type;
    union {
        int i;
        float f;
    } as;
} ULAtomNumber;

typedef struct ULAtomString_ {
    ULAtomHeader header;
    ULStr str;
} ULAtomString;

typedef struct ULAtomSymbol_ {
    ULAtomHeader header;
    ULStr id;
} ULAtomSymbol;

typedef struct ULAtomCons_ {
    ULAtomHeader header;
    ULAtom car;
    ULAtom cdr;
} ULAtomCons;

typedef union ULAtomData_ {
    ULAtomHeader header;
    ULAtomFree free;
    ULAtomError error;
    ULAtomNumber number;
    ULAtomString string;
    ULAtomSymbol symbol;
    ULAtomCons cons;
} ULAtomData;

#define UL_POOL_MAX (1024)
typedef struct ULPool_ ULPool;

struct ULPool_ {
    ULAtomData e[UL_POOL_MAX];
    ULPool *next;
};

typedef struct ULCore_ {
    ULPool *pool;
    ULAtomFree *free;
    bool initialized;
} ULCore;

extern ULCore UL_CORE;

ULAtom ul_error(void);
ULAtom ul_error_oom(void);
    
size_t ul_grow_capacity(size_t old_capacity, size_t init_capacity);
    
void *ul_malloc(size_t size);
void *ul_realloc(void *ptr, size_t new_size);
void ul_free(void *ptr);

ULAtom ul_pool_expand(void);
ULAtom ul_atom_alloc(void);

void ul_writer_deinit(ULWriter *w);
ULAtom ul_writer_expand(ULWriter *w, size_t needed);
ULAtom ul_writer_char(ULWriter *w, char c);
ULAtom ul_writer_string(ULWriter *w, ULStr string);
ULAtom ul_writer_string_escaped(ULWriter *w, ULStr string);
UL_PRINTF_FMT(2, 3) ULAtom ul_writer_fmt(ULWriter *w, const char *fmt, ...);
ULAtom ul_writer_int(ULWriter *w, int integer);
ULAtom ul_writer_float(ULWriter *w, float floater);
ULAtom ul_writer_address(ULWriter *w, size_t address);
ULAtom ul_writer_newline(ULWriter *w);
ULStr ul_writer_slice(ULWriter *w);
ULAtom ul_writer_atom(ULWriter *w, ULAtom atom);
    
ULAtom ul_str_dupe(ULStr input, ULStr *output);
bool ul_str_eq(ULStr a, ULStr b);
void ul_str_reverse_mut(char *c, size_t length);

ul_u8 ul_char_to_digit(char c);
char ul_char_from_digit(ul_u8 integer);
bool ul_char_is_digit(char c);
bool ul_char_is_whitespace(char c);
bool ul_char_is_alphabet(char c);
bool ul_char_is_symbol(char c);
char ul_char_from_hex(ul_u8 byte);
bool ul_char_is_hex(char c);
ul_u8 ul_char_to_hex(char c);

size_t ul_hash_djb2(const char *c, size_t length);
    
#endif // ULISP_INTERNALS_H
