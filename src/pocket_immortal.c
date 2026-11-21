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
