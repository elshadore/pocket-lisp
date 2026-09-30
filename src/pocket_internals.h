#ifndef POCKET_INTERNALS_H
#define POCKET_INTERNALS_H

#include "pocket.h"

typedef int8_t pk_i8;
typedef int16_t pk_i16;
typedef int32_t pk_i32;
typedef int64_t pk_i64;

typedef uint8_t pk_u8;
typedef uint16_t pk_u16;
typedef uint32_t pk_u32;
typedef uint64_t pk_u64;

typedef size_t pk_usize;
typedef ssize_t pk_isize;

#define PK_ATOMTY_ILLEGAL (0)
#define PK_ATOMTY_TYPE (1)
#define PK_ATOMTY_FREE (2)
#define PK_ATOMTY_ERROR (3)
#define PK_ATOMTY_NIL (4)
#define PK_ATOMTY_NUMBER (5)
#define PK_ATOMTY_STRING (6)
#define PK_ATOMTY_SYMBOL (7)
#define PK_ATOMTY_KEYWORD (8)
#define PK_ATOMTY_CONS (9)
#define PK_ATOMTY_ARRAY (10)
#define PK_ATOMTY_TABLE (11)
#define PK_ATOMTY_INTERPRETTER (12)

typedef struct PKAtomHeader_ {
    pk_u8 type;
    pk_usize refcount;
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

typedef union PKAtomData_ {
    PKAtomHeader header;
    PKAtomFree free;
    PKAtomError error;
    PKAtomNumber number;
} PKAtomData;

#define PK_POOL_MAX (1024)
typedef struct PKPool_ PKPool;

struct PKPool_ {
    PKAtomData e[PK_POOL_MAX];
    PKPool *next;
};

typedef struct PocketCore_ {
    PKPool *pool;
    PKAtomFree *free;
    bool initialized;
} PocketCore;

extern PocketCore PK_CORE;

void *pk_malloc(size_t size);
void *pk_realloc(void *ptr, size_t new_size);
void pk_free(void *ptr);

bool pk_pool_expand(void);
PKAtom pk_atom_alloc(void);

#endif // POCKET_INTERNALS_H
