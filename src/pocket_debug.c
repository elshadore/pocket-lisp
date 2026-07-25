#include "pocket_internals.h"

PK_RES pk_dump_stack(Pocket lisp, const char *tag) {
    int top = pk_get_top(lisp);
    int i = 0;
    PKWriter w = pk_writer_init(lisp);
    PK_RES result = PK_YIELD;
    
    pk_defer(pk_writer_string(&w, "*~STACK-DUMP~*"));
    if (tag != NULL) {
        pk_defer(pk_writer_string(&w, " (tag = "));
        pk_defer(pk_writer_string(&w, tag));
        pk_defer(pk_writer_string(&w, ")"));
    }
    pk_defer(pk_writer_newline(&w));
    
    for (i = top; i > 0; i--) {
        int rel = pk_sp_relative(lisp, i);
        PKAtom *a = NULL;
        pk_defer(pk_stack_get(lisp, i, &a));
        
        pk_defer(pk_writer_string(&w, "    ["));
        pk_defer(pk_writer_int(&w, rel));
        pk_defer(pk_writer_string(&w, "/"));
        pk_defer(pk_writer_int(&w, i));
        pk_defer(pk_writer_string(&w, "] => "));
        
        pk_defer(pk_writer_atom(&w, a));
        pk_defer(pk_writer_newline(&w));
    }
    pk_writer_puts(&w);
    result = PK_OK;
    DEFER:
    pk_writer_deinit(&w);
    return result;
}

PK_RES pk_dump_hex_atom(Pocket lisp, PKAtomLFunc *lfunc) {
    PKWriter w = pk_writer_init(lisp);
    PK_RES result = PK_YIELD;
    size_t i = 0;
    
    pk_defer(pk_writer_string(&w, "*~HEX-DUMP~*\n"));
    pk_defer(pk_writer_string(&w, "SECTION-CODE:\n"));
    for (i = 0; i < lfunc->bc.length; ++i) {
        pk_u8 byte = lfunc->bc.e[i];
        const char *ident = pk_ident_opcode(byte);
        PK_OPCODE_TY ty = pk_opcode_ty(byte);
        pk_defer(pk_writer_string(&w, "["));
        pk_defer(pk_writer_int(&w, (int)i));
        pk_defer(pk_writer_string(&w, "] => "));
        /*
        pk_defer(pk_writer_string(&w, "["));
        pk_defer(pk_writer_int(&w, (int)byte));
        pk_defer(pk_writer_string(&w, "] => "));
        */
        pk_defer(pk_writer_string(&w, ident));
        switch (ty) {
            case PK_OPCODE_TY_LOAD: {
                pk_u8 data = 0;
                PKAtom *atom = NULL;
                i++;
                if (i >= lfunc->bc.length) {
                    pk_defer(pk_error(lisp));
                }
                data = lfunc->bc.e[i];
                atom = lfunc->atoms.e[data];
                
                pk_defer(pk_writer_string(&w, " => "));
                pk_defer(pk_writer_atom(&w, atom));
                break;
            }
            case PK_OPCODE_TY_LIT: {
                pk_u8 data = 0;
                i++;
                if (i >= lfunc->bc.length) {
                    pk_defer(pk_error(lisp));
                }
                data = lfunc->bc.e[i];
                
                pk_defer(pk_writer_string(&w, " => "));
                pk_defer(pk_writer_int(&w, (int)data));
                break;
            }
            default: break;
        }
        pk_defer(pk_writer_newline(&w));
    }

    pk_defer(pk_writer_newline(&w));
    
    pk_defer(pk_writer_string(&w, "SECTION-DATA:\n"));
    for (i = 0; i < lfunc->atoms.length; ++i) {
        pk_defer(pk_writer_atom(&w, lfunc->atoms.e[i]));
        pk_defer(pk_writer_newline(&w));
    }
    
    pk_defer(pk_writer_newline(&w));
    
    pk_writer_puts(&w);
    result = PK_OK;
    
    DEFER:
    pk_writer_deinit(&w);
    return result;
}

PK_RES pk_dump_hex(Pocket lisp, int stack_pointer) {
    PKAtom *atom = NULL;
    PKAtomLFunc *lfunc = NULL;

    pk_try(pk_stack_get(lisp, stack_pointer, &atom));
    pk_try(pk_atom_cast_lfunc(lisp, atom, &lfunc));
    pk_try(pk_dump_hex_atom(lisp, lfunc));

    return PK_OK;
}

PK_RES pk_dump_env(Pocket lisp, const char *tag) {
    PKWriter w = pk_writer_init(lisp);
    PK_RES result = PK_YIELD;
    size_t i = 0;
    
    pk_defer(pk_writer_string(&w, "*~ENVIRONMENT~*"));
    if (tag != NULL) {
        pk_defer(pk_writer_string(&w, " (tag = "));
        pk_defer(pk_writer_string(&w, tag));
        pk_defer(pk_writer_string(&w, ")"));
    }
    pk_defer(pk_writer_newline(&w));
    
    pk_defer(pk_writer_string(&w, "SECTION: VARS\n"));
    for (i = 0; i < lisp->vars.capacity; i++) {
        PKSymTableSlot *slot = NULL;
        for (slot = lisp->vars.e[i]; slot; slot = slot->chain) {
            pk_defer(pk_writer_string(&w, "    ["));
            pk_defer(pk_writer_atom(&w, (PKAtom *)slot->key));
            pk_defer(pk_writer_string(&w, "] => "));
            pk_defer(pk_writer_atom(&w, slot->value));
            pk_defer(pk_writer_newline(&w));
        }
    }
    
    pk_defer(pk_writer_string(&w, "SECTION: FUNS\n"));
    for (i = 0; i < lisp->funs.capacity; i++) {
        PKSymTableSlot *slot = NULL;
        for (slot = lisp->funs.e[i]; slot; slot = slot->chain) {
            pk_defer(pk_writer_string(&w, "    ["));
            pk_defer(pk_writer_atom(&w, (PKAtom *)slot->key));
            pk_defer(pk_writer_string(&w, "] => "));
            pk_defer(pk_writer_atom(&w, slot->value));
            pk_defer(pk_writer_newline(&w));
        }
    }
    
    pk_writer_print(&w);
    
    result = PK_OK;
    DEFER:
    pk_writer_deinit(&w);
    return result;
}

PK_RES pk_dump_frame__(Pocket lisp, PKWriter *w, PKFrame *frame, size_t length, size_t index) {
    PKAtomSlice atoms;
    size_t i = 0;
    
    pk_try(pk_writer_string(w, "FRAME: ["));
    pk_try(pk_writer_int(w, (int)index));
    pk_try(pk_writer_string(w, "]\n"));
    
    atoms = pk_frame_slice(lisp, frame, length);
    for (i = 0; i < atoms.length; ++i) {
        size_t index = pk_index_inv(i, atoms.length);
        int a = (int)(index + 1);
        pk_try(pk_writer_string(w, "    ["));
        pk_try(pk_writer_int(w, -a));
        pk_try(pk_writer_string(w, " / "));
        pk_try(pk_writer_int(w, a));
        pk_try(pk_writer_string(w, "] => "));
        pk_try(pk_writer_atom(w, atoms.e[index]));
        pk_try(pk_writer_newline(w));
    }
    return PK_OK;
}

PK_RES pk_dump_trace(Pocket lisp, const char *tag) {
    PKWriter w = pk_writer_init(lisp);
    PK_RES result = PK_YIELD;
    size_t length = 0;
    size_t dec = 0;
    size_t i = 0;
    
    pk_defer(pk_writer_string(&w, "*~TRACE-DUMP~*"));
    if (tag != NULL) {
        pk_defer(pk_writer_string(&w, " (tag = "));
        pk_defer(pk_writer_string(&w, tag));
        pk_defer(pk_writer_string(&w, ")"));
    }
    pk_defer(pk_writer_newline(&w));
    
    length = pk_stack_length_frame(lisp);
    dec = pk_stack_length_total(lisp) - length;
    
    pk_defer(pk_dump_frame__(lisp, &w, &lisp->current_frame, length, lisp->frames.count));
    
    for (i = 0; i < lisp->frames.count; ++i) {
        size_t index = pk_index_inv(i, lisp->frames.count);
        PKFrame frame = lisp->frames.e[index];
        size_t length = dec - frame.stack_offset;
        pk_defer(pk_dump_frame__(lisp, &w, &frame, length, index));
        dec -= length;
    }
    
    pk_writer_print(&w);
    
    result = PK_OK;
    DEFER:
    pk_writer_deinit(&w);
    return result;
}

