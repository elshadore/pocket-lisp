#include "pocket_internals.h"

void pk_temp_panic(void) {
    fprintf(stderr, "TODO: reimplement throwing and errors");
    abort();
}

PKAtom pk_throw(PKAtom type, PKAtom contents) {
    (void)type;
    (void)contents;
    pk_temp_panic();
    return pk_true();
}

PKAtom pk_error(PKAtom message) {
    return pk_throw(PK_CORE.cache.error, message);
}

PKAtom pk_errors(PKStr message) {
    PKAtom atom = pk_string(message);
    pk_propagate(atom);
    PKAtom error = pk_error(atom);
    pk_ref_dec(atom);
    return error;
}

PKAtom pk_error_type(PK_TYPE expected, PKAtom found) {
    PKWriter w = PK_WRITER_EMPTY;
    pk_propagate(pk_writer_string(&w, pk_str("expected ")));
    pk_propagate(pk_writer_string(&w, pk_ident_type(expected)));
    pk_propagate(pk_writer_string(&w, pk_str(", got ")));
    pk_propagate(pk_writer_atom(&w, found));
    PKAtom message = pk_string(pk_writer_slice(&w));
    pk_writer_deinit(&w);
    pk_propagate(message);
    PKAtom error = pk_error(message);
    pk_ref_dec(message);
    return error;
}
    
PKAtom pk_error_oom(void) {
    pk_temp_panic();
    return pk_true();
}

PKAtom pk_error_corrupted_memory(void) {
    pk_temp_panic();
    return pk_true();
}

PKAtom pk_error_refcount_overflow(void) {
    pk_temp_panic();
    return pk_true();
}
