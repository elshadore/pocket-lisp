#include "pocket_internals.h"

FILE *pk_fs_file_open(Pocket lisp, PKStr file_path, const char *mode) {
    if (file_path.length == 0) {
        pk_errors(lisp, pk_str("empty file path"));
    }

    char *cfile_path = (char *)pk_malloc(lisp, file_path.length * sizeof(char));
    
    (void)memcpy(cfile_path, file_path.c, file_path.length);
    cfile_path[file_path.length] = '\0';

    FILE *result = fopen(cfile_path, mode);
    pk_free(lisp, cfile_path);
    
    if (result == NULL) {
        pk_errorf(
            lisp,
            "failed to open file: %.*s, with mode: %s",
            (int)file_path.length,
            file_path.c,
            mode
        );
    }

    return result;
}

void pk_fs_file_splat_str(Pocket lisp, PKStr file_path, PKStr input) {
    (void)lisp;
    FILE *file = pk_fs_file_open(lisp, file_path, "w");
    fwrite(input.c, sizeof(char), input.length, file);
    fclose(file);
}

void pk_fs_file_splat(Pocket lisp, PKAtomString *file_path, PKAtomString *input) {
    pk_fs_file_splat_str(lisp, file_path->str, input->str);
}

PKAtomString *pk_fs_file_slurp_str(Pocket lisp, PKStr file_path) {
    FILE *file = pk_fs_file_open(lisp, file_path, "r");
    
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

PKAtomString *pk_fs_file_slurp(Pocket lisp, PKAtomString *file_path) {
    return pk_fs_file_slurp_str(lisp, file_path->str);
}

PKAtom *pk_fs_read_user_input(Pocket lisp) {
    pk_writer_reset(lisp);
    for (;;) {
        int c = getc(stdin);
        if (c == EOF) {
            if (pk_writer_str(lisp).length == 0) {
                return lisp->cache.nil;
            }
            break;
        }
        if (c == '\n') {
            break;
        }
        pk_writer_char(lisp, (char)c);
    }

    PKAtom *result = (PKAtom *)pk_atom_string(lisp, pk_writer_str(lisp));
    
    pk_writer_reset(lisp);
    
    return result;
}
