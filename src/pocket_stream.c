#include "pocket_internals.h"

typedef struct PKOpenModeEntry_ {
    PKStr name;
    PK_OPEN_MODE mode;
} PKOpenModeEntry;

typedef struct PKStreamReadData_ {
    PKAtomStream *stream;
    size_t amount;
    PK_READ_MODE mode;
    size_t count;
    PKBytes b;
    PKWriter string;
    PKAtom *result;
} PKStreamReadData;

const char *pk_open_mode_to_str(PK_OPEN_MODE mode) {
    switch (mode) {
        case PK_OPEN_MODE_READ: return "r";
        case PK_OPEN_MODE_WRITE: return "w";
        case PK_OPEN_MODE_APPEND: return "a";
        case PK_OPEN_MODE_READ_WRITE: return "w+";
        case PK_OPEN_MODE_READ_APPEND: return "a+";
        default: return NULL;
    }
}

PKStr pk_atom_stream_mode_name(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_keyword(atom)) {
        return ((PKAtomKeyword *)atom)->id;
    }
    if (pk_atom_is_symbol(atom)) {
        return ((PKAtomSymbol *)atom)->name->str;
    }
    if (pk_atom_is_string(atom)) {
        return ((PKAtomString *)atom)->str;
    }
    if (pk_atom_is_nil(atom)) {
        return pk_str("read-write");
    }
    pk_error_type(lisp, atom, lisp->cache.type_keyword);
    return PK_STR_EMPTY;
}

PK_OPEN_MODE pk_open_mode_from_str(Pocket lisp, PKStr name) {
    PKOpenModeEntry kinds[] = {
        {pk_str("read"), PK_OPEN_MODE_READ},
        {pk_str("write"), PK_OPEN_MODE_WRITE},
        {pk_str("append"), PK_OPEN_MODE_APPEND},
        {pk_str("read-write"), PK_OPEN_MODE_READ_WRITE},
        {pk_str("read-append"), PK_OPEN_MODE_READ_APPEND},
    };

    size_t length = pk_alen(kinds);

    for (size_t i = 0; i < length; ++i) {
        if (pk_str_eq(kinds[i].name, name)) {
            return kinds[i].mode;
        }
    }

    pk_errorf(lisp, "unknown stream open mode: %.*s", (int)name.length, name.c);
    return PK_OPEN_MODE_READ;
}

PKAtomStream *pk_atom_stream_inherit(Pocket lisp, FILE *file, bool keep_open) {
    PKAtomStream *atom = (PKAtomStream *)pk_atom_alloc(lisp);
    atom->header.type = PK_ATOMTY_STREAM;
    atom->f = file;
    atom->keep_open = keep_open;

    return atom;
}

PKAtomStream *pk_atom_stream_file_open(Pocket lisp, PKStr file_path, PK_OPEN_MODE mode) {
    if (file_path.length == 0) {
        pk_errors(lisp, pk_str("empty file path"));
    }

    const char *cmode = pk_open_mode_to_str(mode);
    if (cmode == NULL) {
        pk_errors(lisp, pk_str("invalid stream open mode"));
    }

    char *cfile_path = (char *)pk_malloc(lisp, (file_path.length + 1) * sizeof(char));
    if (cfile_path == NULL) {
        pk_free(lisp, cfile_path);
        pk_errors(lisp, pk_str("out of memory"));
    }
    (void)memcpy(cfile_path, file_path.c, file_path.length);
    cfile_path[file_path.length] = '\0';

    FILE *file = fopen(cfile_path, cmode);
    pk_free(lisp, cfile_path);

    if (file == NULL) {
        pk_errors(lisp, pk_str("failed to open file stream"));
    }

    return pk_atom_stream_inherit(lisp, file, false);
}

PKAtom *pk_atom_stream_read_char(Pocket lisp, PKAtomStream *stream) {
    int c = fgetc(stream->f);
    if (c == EOF) {
        return lisp->cache.nil;
    }

    return (PKAtom *)pk_atom_char(lisp, (char)c);
}

PKAtom *pk_atom_stream_read_byte(Pocket lisp, PKAtomStream *stream) {
    int c = fgetc(stream->f);
    if (c == EOF) {
        return lisp->cache.nil;
    }

    return (PKAtom *)pk_atom_byte(lisp, (pk_u8)c);
}

void pk_atom_stream_read_body(void *user_closure, Pocket lisp) {
    PKStreamReadData *data = (PKStreamReadData *)user_closure;

    data->count = 0;

    for (size_t i = 0; i < data->amount; ++i) {
        int c = fgetc(data->stream->f);
        if (c == EOF) {
            break;
        }
        if (ferror(data->stream->f)) {
            pk_errors(lisp, pk_str("stream read failed"));
        }
        if (data->mode == PK_READ_MODE_BINARY) {
            pk_bytes_push(lisp, &data->b, (pk_u8)c, 16);
        } else {
            pk_writer_char(lisp, &data->string, (char)c);
        }
        data->count += 1;
    }

    if (data->count == 0) {
        data->result = lisp->cache.nil;
        pk_stack_push(lisp, data->result);
        return;
    }

    if (data->mode == PK_READ_MODE_BINARY) {
        data->result = (PKAtom *)pk_atom_blob(lisp, pk_bytes_slice(&data->b));
    } else {
        data->result = (PKAtom *)pk_atom_string(lisp, pk_writer_slice(&data->string));
    }
    pk_stack_push(lisp, data->result);
}

PKAtom *pk_atom_stream_read(Pocket lisp, PKAtomStream *stream, size_t amount, PK_READ_MODE mode) {
    PKStreamReadData data = (PKStreamReadData) {
        .stream = stream,
        .amount = amount,
        .mode = mode,
        .count = 0,
        .b = PK_BYTES_EMPTY,
        .string = PK_WRITER_EMPTY,
        .result = NULL,
    };
    
    PK_RES result = pk_protect(lisp, &data, pk_atom_stream_read_body, 0);

    pk_bytes_deinit(lisp, &data.b);
    pk_writer_deinit(lisp, &data.string);

    if (result == PK_YIELDED) {
        pk_propagate(lisp);
    }

    return pk_stack_head_pop(lisp);
}

PKAtom *pk_atom_stream_read_atom(Pocket lisp, PKAtomStream *stream, size_t amount, PKAtom *mode) {
    PK_READ_MODE tmode = PK_READ_MODE_STRING;
    if (pk_atom_text_qeq(lisp, mode, pk_str("string"))) {
        tmode = PK_READ_MODE_STRING;
    }
    if (pk_atom_text_qeq(lisp, mode, pk_str("text"))) {
        tmode = PK_READ_MODE_STRING;
    }
    if (pk_atom_text_qeq(lisp, mode, pk_str("binary"))) {
        tmode = PK_READ_MODE_BINARY;
    }
    return pk_atom_stream_read(lisp, stream, amount, tmode);
}

void pk_atom_stream_write_string(Pocket lisp, PKAtomStream *stream, PKStr string) {
    size_t written = fwrite(string.c, 1, string.length, stream->f);
    if (written != string.length) {
        pk_errors(lisp, pk_str("stream write failed"));
    }
}

void pk_atom_stream_write_bytes(Pocket lisp, PKAtomStream *stream, PKBlob blob) {
    if (blob.length == 0) {
        return;
    }

    size_t written = fwrite(blob.e, 1, blob.length, stream->f);
    if (written != blob.length) {
        pk_errors(lisp, pk_str("stream write failed"));
    }
}

void pk_atom_stream_write(Pocket lisp, PKAtomStream *stream, PKAtom *atom) {
    switch (atom->header.type) {
        case PK_ATOMTY_STRING: {
            pk_atom_stream_write_string(lisp, stream, atom->string.str);
            break;
        }
        case PK_ATOMTY_CHARACTER: {
            PKStr string = (PKStr) {
                .c = &atom->character.c,
                .length = 1,
            };
            pk_atom_stream_write_string(lisp, stream, string);
            break;
        }
        case PK_ATOMTY_BLOB: {
            pk_atom_stream_write_bytes(lisp, stream, atom->blob.bytes);
            break;
        }
        case PK_ATOMTY_NUMBER: {
            pk_u8 byte = pk_atom_to_byte(lisp, atom);
            PKBlob blob = (PKBlob) {
                .e = &byte,
                .length = 1,
            };
            pk_atom_stream_write_bytes(lisp, stream, blob);
            break;
        }
        default: {
            pk_errors(lisp, pk_str("cannot write this type"));
            break;
        }
    }
}

void pk_atom_stream_flush(Pocket lisp, PKAtomStream *stream) {
    if (fflush(stream->f) != 0) {
        pk_errors(lisp, pk_str("stream flush failed"));
    }
}

bool pk_atom_is_stream(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_STREAM;
}

PKAtomStream *pk_atom_assert_stream(Pocket lisp, PKAtom *atom) {
    if (!pk_atom_is_stream(atom)) {
        pk_error_type(lisp, atom, lisp->cache.type_stream);
    }
    return (PKAtomStream *)atom;
}
