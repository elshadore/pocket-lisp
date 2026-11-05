#include "pocket_internals.h"

void pk_internal_print(Pocket lisp, PKStr print) {
    (void)lisp;
    fprintf(stderr, "%.*s", (int)print.length, print.c);
    fflush(stderr);
}

void pk_internal_println(Pocket lisp, PKStr print) {
    (void)lisp;
    fprintf(stderr, "%.*s\n", (int)print.length, print.c);
    fflush(stderr);
}

typedef struct PKAtomPrintData_ {
    PKWriter w;
    PKAtom *atom;
} PKAtomPrintData;

void pk_atom_print_body(void *user_closure, Pocket lisp) {
    PKAtomPrintData *data = (PKAtomPrintData *)user_closure;
    pk_writer_atom(lisp, &data->w, data->atom);
    pk_internal_println(lisp, pk_writer_slice(&data->w));
}

void pk_atom_print(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_string(atom)) {
        PKAtomString *string = (PKAtomString *)atom;
        pk_internal_println(lisp, string->str);
        return;
    }

    PKAtomPrintData data = (PKAtomPrintData) {
        .w = PK_WRITER_EMPTY,
        .atom = atom,
    };

    PK_RES result = pk_protect_noreturn(lisp, &data, pk_atom_print_body, 0);
    pk_writer_deinit(lisp, &data.w);

    if (result == PK_YIELDED) {
        pk_propagate(lisp);
    }
}

typedef struct PKAtomWriteData_ {
    PKWriter w;
    PKAtom *atom;
} PKAtomWriteData;

void pk_atom_write_body(void *user_closure, Pocket lisp) {
    PKAtomWriteData *data = (PKAtomWriteData *)user_closure;
    pk_writer_atom(lisp, &data->w, data->atom);
    pk_internal_print(lisp, pk_writer_slice(&data->w));
}

void pk_atom_write(Pocket lisp, PKAtom *atom) {
    if (pk_atom_is_string(atom)) {
        PKAtomString *string = (PKAtomString *)atom;
        pk_internal_print(lisp, string->str);
        return;
    }

    PKAtomWriteData data = (PKAtomWriteData) {
        .w = PK_WRITER_EMPTY,
        .atom = atom,
    };

    PK_RES result = pk_protect_noreturn(lisp, &data, pk_atom_write_body, 0);
    pk_writer_deinit(lisp, &data.w);

    if (result == PK_YIELDED) {
        pk_propagate(lisp);
    }
}

typedef struct PKAtomFormatData_ {
    PKWriter w;
    PKAtom *atom;
} PKAtomFormatData;

void pk_atom_format_body(void *user_closure, Pocket lisp) {
    PKAtomFormatData *data = (PKAtomFormatData *)user_closure;
    pk_writer_atom(lisp, &data->w, data->atom);
    pk_stack_push(lisp, (PKAtom *)pk_atom_string(lisp, pk_writer_slice(&data->w)));
}

PKAtomString *pk_atom_format(Pocket lisp, PKAtom *atom) {
    PKAtomFormatData data = (PKAtomFormatData) {
        .w = PK_WRITER_EMPTY,
        .atom = atom,
    };

    PK_RES result = pk_protect(lisp, &data, pk_atom_format_body, 0);

    pk_writer_deinit(lisp, &data.w);

    if (result == PK_YIELDED) {
        pk_propagate(lisp);
    }

    PKAtom *out = pk_stack_head_pop(lisp);
    return (PKAtomString *)out;
}
