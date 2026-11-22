#include "pocket_internals.h"

PKAtom PK_ATOM_NIL = (PKAtom) {
    .header.type = PK_ATOMTY_NIL,
    .header.flags = PK_ATOMFLAG_IMMORTAL,
};

PKAtom PK_ATOM_TRUE = (PKAtom) {
    .header.type = PK_ATOMTY_TRUE,
    .header.flags = PK_ATOMFLAG_IMMORTAL,
};

PKAtom PK_ATOM_UNDEFINED = (PKAtom) {
    .header.type = PK_ATOMTY_UNDEFINED,
    .header.flags = PK_ATOMFLAG_IMMORTAL,
};

PKAtomString PK_ATOM_EMPTY_STRING = (PKAtomString) {
    .header.type = PK_ATOMTY_STRING,
    .header.flags = PK_ATOMFLAG_IMMORTAL,
    .str = PK_STR_EMPTY,
};

PKAtomString PK_ATOM_OUT_OF_MEMORY = (PKAtomString) {
    .header.type = PK_ATOMTY_STRING,
    .header.flags = PK_ATOMFLAG_IMMORTAL,
    .str = pk_str("out of memory"),
};

PKAtomBlob PK_ATOM_EMPTY_BLOB = (PKAtomBlob) {
    .header.type = PK_ATOMTY_BLOB,
    .header.flags = PK_ATOMFLAG_IMMORTAL,
    .bytes = PK_BLOB_EMPTY,
};

PKAtomString PK_ATOM_STRING_CACHE[128];
PKAtomNumber PK_ATOM_NUMBER_CACHE[101];

static char PK_CHAR_CACHE[128];

void pk_immortal_init(void) {
    for (int i = 0; i < 128; ++i) {
        PK_CHAR_CACHE[i] = (char)i;
        PK_ATOM_STRING_CACHE[i].header.type = PK_ATOMTY_STRING;
        PK_ATOM_STRING_CACHE[i].header.flags = PK_ATOMFLAG_IMMORTAL;
        PK_ATOM_STRING_CACHE[i].str = (PKStr) {
            .c = &PK_CHAR_CACHE[i],
            .length = 1,
        };
    }

    for (int i = 0; i <= 100; ++i) {
        PK_ATOM_NUMBER_CACHE[i].header.type = PK_ATOMTY_NUMBER;
        PK_ATOM_NUMBER_CACHE[i].header.flags = PK_ATOMFLAG_IMMORTAL;
        PK_ATOM_NUMBER_CACHE[i].num_type = PK_NUMTY_INT;
        PK_ATOM_NUMBER_CACHE[i].as.i = i;
    }
}
