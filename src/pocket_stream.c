#include "pocket_internals.h"

PKAtomStream *pk_atom_stream_file_open(Pocket lisp, PKStr file_path) {
    if (file_path.length == 0) {
        pk_errors(lisp, pk_str("empty file path"));
    }
    
    char *cfile_path = (char *)pk_malloc(lisp, (file_path.length + 1) * sizeof(char));
    if (cfile_path == NULL) {
        pk_free(lisp, cfile_path);
        pk_errors(lisp, pk_str("out of memory"));
    }
    (void)memcpy(cfile_path, file_path.c, file_path.length);
    cfile_path[file_path.length] = '\0';

    FILE *file = fopen(cfile_path, "r");
    pk_free(lisp, cfile_path);
    
    if (file == NULL) {
        pk_errors(lisp, pk_str("failed to open file stream"));
    }

    PKAtomStream *atom = (PKAtomStream *)pk_atom_alloc(lisp);
    atom->header.type = PK_ATOMTY_STREAM;
    atom->f = file;

    return atom;
}

char pk_atom_stream_read_char(Pocket lisp, PKAtomStream *stream) {
    
}

pk_u8 pk_atom_stream_read_byte(Pocket lisp, PKAtomStream *stream) {
    
}

PKAtom *pk_atom_stream_read(Pocket lisp, PKAtomStream *stream, size_t amount, PK_READ_MODE mode) {

}

void pk_atom_stream_write_string(Pocket lisp, PKAtomStream *stream, PKStr string) {
    
}

void pk_atom_stream_write_bytes(Pocket lisp, PKAtomStream *stream, PKBlob blob) {
    
}
