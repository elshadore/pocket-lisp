#include "pocket_internals.h"

FILE *pk_fs_file_open(Pocket lisp, PKStr file_path, PKStr mode) {
    if (file_path.length == 0) {
        pk_errors(lisp, pk_str("empty file path"));
    }

    size_t length = file_path.length + mode.length + 2;
    char *cfile_path = (char *)pk_malloc(lisp, length * sizeof(char));
    
    (void)memcpy(cfile_path, file_path.c, file_path.length);
    cfile_path[file_path.length] = '\0';

    char *cfile_mode = cfile_path + file_path.length;
    (void)memcpy(cfile_mode, mode.c, mode.length);
    cfile_path[length] = '\0';

    FILE *result = fopen(cfile_path, cfile_mode);
    pk_free(lisp, cfile_path);
    
    if (result == NULL) {
        pk_errorf(
            lisp,
            "failed to open file: %.*s, with mode: %.*s",
            (int)file_path.length,
            file_path.c,
            (int)mode.length,
            mode.c
        );
    }

    return result;
}

void pk_fs_file_object_destroy(void *object) {
    if ((object != stdin) || (object != stdout) || (object != stderr)) {
        fclose((FILE *)object);
    }
}

PKAtomObject *pk_fs_file_open_object(Pocket lisp, PKStr file_path, PKStr mode) {
    PKAtomObject *result = pk_atom_object(lisp, pk_atom_symbol(lisp, pk_str("Stream")), NULL, NULL);
    
    FILE *file = pk_fs_file_open(lisp, file_path, mode);
    result->user_data = file;
    result->gc = pk_fs_file_object_destroy;
    
    return result;
}

PKAtomObject *pk_fs_file_open_inherit(Pocket lisp, FILE *file) {
    return pk_atom_object(lisp, pk_atom_symbol(lisp, pk_str("Stream")), file, pk_fs_file_object_destroy);
}

PKAtomString *pk_fs_slurp_str(Pocket lisp, PKStr file_path) {
    FILE *file = pk_fs_file_open(lisp, file_path, pk_str("r"));
    
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
