#include "pocket_internals.h"

PKAtom pk_debug_dump_(PKWriter *w, const char *tag, PK_DUMP opt) {
    pk_propagate(pk_writer_string(w, pk_str("*~DEBUG-DUMP~*")));
    if (tag != NULL) {
        pk_propagate(pk_writer_string(w, pk_str(" (tag = ")));
        pk_propagate(pk_writer_cstr(w, tag));
        pk_propagate(pk_writer_string(w, pk_str(")")));
    }
    
    pk_propagate(pk_writer_newline(w));

    if ((opt & PK_DUMP_STACK) != 0) {
        pk_propagate(pk_writer_string(w, pk_str("STACK: ")));
        pk_propagate(pk_writer_fmt(w, "%zu", PK_CORE.current_frame.stack_offset));
        pk_propagate(pk_writer_newline(w));

        for (size_t i = 0; i < PK_CORE.stack.count; ++i) {
            size_t index = PK_CORE.stack.count - i - 1;
            pk_propagate(pk_writer_string(w, pk_str("    [")));
            pk_propagate(pk_writer_fmt(w, "%zu", index));
            pk_propagate(pk_writer_string(w, pk_str("] => ")));
            PKAtom atom = PK_CORE.stack.e[index];
            pk_propagate(pk_writer_atom(w, atom));
            pk_propagate(pk_writer_newline(w));
        }
    }
    if ((opt & PK_DUMP_ENV) != 0){
        pk_propagate(pk_writer_string(w, pk_str("ENV:")));
        pk_propagate(pk_writer_newline(w));
        size_t acc = 0;
        for (PKPool *iter = PK_CORE.pool; iter != NULL; iter = iter->next) {
            for (size_t i = 0; i < PK_POOL_MAX; ++i) {
                PKAtom symbol = &iter->e[i];
                if (pk_is_symbol(symbol)) {
                    pk_propagate(pk_writer_string(w, pk_str("    [")));
                    pk_propagate(pk_writer_fmt(w, "%zu", acc));
                    pk_propagate(pk_writer_string(w, pk_str("] => ")));
                    pk_propagate(pk_writer_atom(w, symbol));
                    pk_propagate(pk_writer_newline(w));
                    pk_propagate(pk_writer_string(w, pk_str("    ")));
                    pk_propagate(pk_writer_atom(w, symbol->symbol.var));
                    pk_propagate(pk_writer_newline(w));
                    pk_propagate(pk_writer_string(w, pk_str("    ")));
                    pk_propagate(pk_writer_atom(w, symbol->symbol.fun));
                    pk_propagate(pk_writer_newline(w));
                    pk_propagate(pk_writer_string(w, pk_str("    ")));
                    pk_propagate(pk_writer_atom(w, symbol->symbol.prop));
                    pk_propagate(pk_writer_newline(w));
                    acc += 1;
                }
            }
        }
    }

    pk_print_str(pk_writer_slice(w));
    
    return pk_true();
}
    
PKAtom pk_debug_dump(const char *tag, PK_DUMP opt) {
    PKWriter w = PK_WRITER_EMPTY;
    PKAtom result = pk_debug_dump_(&w, tag, opt);
    pk_writer_deinit(&w);
    return result;
}

PKAtom pk_hexdump_(PKWriter *w, PKAtom lfun) {
    pk_propagate(pk_writer_string(w, pk_str("*~HEX-DUMP~*\n")));
    pk_propagate(pk_writer_string(w, pk_str("SECTION-CODE:\n")));
    
    for (size_t i = 0; i < lfun->lfun.bc.length; ++i) {
        pk_propagate(pk_writer_string(w, pk_str("[")));
        pk_propagate(pk_writer_fmt(w, "%zu", i));
        pk_propagate(pk_writer_string(w, pk_str("] => ")));
        
        pk_u8 byte = lfun->lfun.bc.e[i];
        PKStr ident = pk_ident_opcode(byte);
        pk_propagate(pk_writer_string(w, ident));
        
        PK_OPCODE_TY ty = pk_opcode_ty(byte);
        
        switch (ty) {
            case PK_OPCODE_TY_LOAD: {
                i++;
                if (i >= lfun->lfun.bc.length) {
                    return pk_error();
                }
                pk_u8 data = lfun->lfun.bc.e[i];
                PKAtom atom = lfun->lfun.atoms.e[data];
                
                pk_propagate(pk_writer_string(w, pk_str(" => ")));
                pk_propagate(pk_writer_atom(w, atom));
                break;
            }
            case PK_OPCODE_TY_LIT: {
                i++;
                if (i >= lfun->lfun.bc.length) {
                    return pk_error();
                }
                pk_u8 data = lfun->lfun.bc.e[i];
                
                pk_propagate(pk_writer_string(w, pk_str(" => ")));
                pk_propagate(pk_writer_fmt(w, "%u", data));
                break;
            }
            default: break;
        }
        pk_propagate(pk_writer_newline(w));
    }

    pk_propagate(pk_writer_newline(w));
    
    pk_propagate(pk_writer_string(w, pk_str("SECTION-DATA:\n")));
    for (size_t i = 0; i < lfun->lfun.atoms.length; ++i) {
        pk_propagate(pk_writer_atom(w, lfun->lfun.atoms.e[i]));
        pk_propagate(pk_writer_newline(w));
    }
    
    pk_propagate(pk_writer_newline(w));
    pk_print_str(pk_writer_slice(w));
    return pk_true();
}
    
PKAtom pk_hexdump(PKAtom lfun) {
    if (!pk_is_lfun(lfun)) {
        return pk_error();
    }
    PKWriter w = PK_WRITER_EMPTY;
    PKAtom result = pk_hexdump_(&w, lfun);
    pk_writer_deinit(&w);
    return result;
}
