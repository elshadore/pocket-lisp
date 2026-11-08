// A bunch of junk functions that we are keeping around for posterity.
// If any one of these break. Just trash them.
// None of these function should be exported/publicised into pocket_internals.h

#include "pocket_internals.h"

PKAtom *pk_atom_reverse(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_STRING: {
            PKStr dupe = pk_str_dupe(lisp, atom->string.str);
            pk_str_reverse_mut(dupe);
            return (PKAtom *)pk_atom_string_nomemcpy(lisp, dupe);
        }
        case PK_ATOMTY_CONS: {
            PKAtom *acc = lisp->cache.nil;
            PKAtom *iter = atom;
            while (pk_atom_is_cons(iter)) {
                acc = (PKAtom *)pk_atom_cons(lisp, iter->cons.car, acc);
                iter = iter->cons.cdr;
            }
            return acc;
        }
        case PK_ATOMTY_NIL: return lisp->cache.nil;
        default: {
            pk_error_type3(lisp, atom, lisp->cache.type_nil, lisp->cache.type_cons, lisp->cache.type_string);
            return lisp->cache.nil;
        }
    }
}

PKAtom *pk_atom_to_list(Pocket lisp, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_NIL: {
            return lisp->cache.nil;
        }
        case PK_ATOMTY_CONS: {
            return atom;
        }
        case PK_ATOMTY_ARRAY: {
            PKAtoms atoms = pk_array_slice(&atom->array.atoms);
            return pk_atom_list(lisp, atoms);
        }
        case PK_ATOMTY_STRING: {
            PKStr str = atom->string.str;
            return pk_atom_to_list_chars(lisp, str);
        }
        case PK_ATOMTY_STRING_ARRAY: {
            PKStr str = pk_writer_slice(&atom->string_array.string);
            return pk_atom_to_list_chars(lisp, str);
        }
        case PK_ATOMTY_TABLE: {
            PKHashmap *map = &atom->table.map;
            return pk_atom_to_list_table(lisp, map);
        }
        case PK_ATOMTY_BLOB: {
            PKBlob bytes = atom->blob.bytes;
            return pk_atom_to_list_bytes(lisp, bytes);
        }
        case PK_ATOMTY_BYTE_ARRAY: {
            PKBlob bytes = pk_bytes_slice(&atom->byte_array.bytes);
            return pk_atom_to_list_bytes(lisp, bytes);
        }
        default: {
            PKStr ty = pk_ident_atomty(atom->header.type);
            pk_errorf(lisp, "pk_atom_to_list: unsupported type %.*s", (int)ty.length, ty.c);
            return lisp->cache.nil;
        }
    }
}

PKAtomString *pk_fs_slurp_str(Pocket lisp, PKStr file_path) {
    if (file_path.length == 0) {
        pk_errors(lisp, pk_str("empty file path"));
    }

    char *cfile_path = (char *)pk_malloc(lisp, (file_path.length + 1) * sizeof(char));
    (void)memcpy(cfile_path, file_path.c, file_path.length);
    cfile_path[file_path.length] = '\0';

    FILE *file = fopen(cfile_path, "r");
    pk_free(lisp, cfile_path);
    if (file == NULL) {
        pk_errors(lisp, pk_str("cannot open file"));
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    if (length < 0) {
        fclose(file);
        pk_errors(lisp, pk_str("cannot get file size"));
    }
    rewind(file);

    PKAtomString *result = pk_atom_string_alloc(lisp, (size_t)length);

    size_t read = fread(result->str.c, 1, (size_t)length, file);
    fclose(file);

    if ((size_t)length != read) {
        pk_errors(lisp, pk_str("cannot read file"));
    }

    return result;
}

PKAtomString *pk_fs_slurp(Pocket lisp, PKAtomString *file_path) {
    return pk_fs_slurp_str(lisp, file_path->str);
}

#define READ_BUFFER_MAX (1024)

char READ_BUFFER[READ_BUFFER_MAX];

PKAtom *pk_fs_read_user_input(Pocket lisp) {
    size_t i = 0;
    for (;;) {
        int c = getc(stdin);
        if (c == EOF) {
            if (i == 0) {
                return lisp->cache.nil;
            }
            break;
        }
        if (c == '\n') {
            break;
        }
        if (i == READ_BUFFER_MAX) {
            pk_errors(lisp, pk_str("input too long"));
        }
        READ_BUFFER[i] = (char)c;
        i += 1;
    }

    PKStr slice = (PKStr){
        .c = READ_BUFFER,
        .length = i,
    };
    return (PKAtom *)pk_atom_string(lisp, slice);
}
