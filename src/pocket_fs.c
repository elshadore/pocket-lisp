#include "pocket_internals.h"

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
