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

void pk_fs_file_write_str(Pocket lisp, PKStr file_path, PKStr input, const char *mode) {
    (void)lisp;
    FILE *file = pk_fs_file_open(lisp, file_path, mode);
    fwrite(input.c, sizeof(char), input.length, file);
    fclose(file);
}

void pk_fs_file_write_bytes(Pocket lisp, PKStr file_path, PKBlob input, const char *mode) {
    (void)lisp;
    FILE *file = pk_fs_file_open(lisp, file_path, mode);
    fwrite(input.e, sizeof(pk_u8), input.length, file);
    fclose(file);
}

void pk_fs_file_write(Pocket lisp, PKAtomString *file_path, PKAtom *input) {
    switch (input->header.type) {
        case PK_ATOMTY_STRING: {
            pk_fs_file_write_str(lisp, file_path->str, input->string.str, "w");
            break;
        }
        case PK_ATOMTY_BLOB: {
            pk_fs_file_write_bytes(lisp, file_path->str, input->blob.bytes, "wb");
            break;
        }
        default: {
            pk_error_type2(lisp, input, PK_TYPE_STRING, PK_TYPE_BLOB);
            break;
        }
    }
}

void pk_fs_file_append(Pocket lisp, PKAtomString *file_path, PKAtom *input) {
    switch (input->header.type) {
        case PK_ATOMTY_STRING: {
            pk_fs_file_write_str(lisp, file_path->str, input->string.str, "a");
            break;
        }
        case PK_ATOMTY_BLOB: {
            pk_fs_file_write_bytes(lisp, file_path->str, input->blob.bytes, "ab");
            break;
        }
        default: {
            pk_error_type2(lisp, input, PK_TYPE_STRING, PK_TYPE_BLOB);
            break;
        }
    }
}

PKAtomString *pk_fs_file_read_string(Pocket lisp, PKAtomString *file_path) {
    FILE *file = pk_fs_file_open(lisp, file_path->str, "r");
    
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

PKAtomBlob *pk_fs_file_read_bytes(Pocket lisp, PKAtomString *file_path) {
    FILE *file = pk_fs_file_open(lisp, file_path->str, "rb");
    
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

    PKAtomBlob *result = pk_atom_blob_alloc(lisp, (size_t)length);

    size_t read = fread(result->bytes.e, 1, (size_t)length, file);
    fclose(file);

    if ((size_t)length != read) {
        pk_errors(lisp, pk_str("cannot read file"));
    }

    return result;
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
