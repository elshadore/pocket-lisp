#include "pocket_internals.h"

PKAtom pk_cfun_cons(void *user_closure, PK_CALL call, void *opaque_fun, pk_u8 args, pk_u8 arity) {
    PKAtom atom = pk_atom_alloc();
    pk_propagate(atom);

    if (call == PK_CALL_FUN) {
        atom->header.type = PK_ATOMTY_CFUN;
    } else {
        atom->header.type = PK_ATOMTY_CMACRO;
    }
    atom->cfun.user_closure = user_closure;
    atom->cfun.args = args;
    atom->cfun.arity = arity;
    atom->cfun.as.opaque = opaque_fun;
    return atom;
}

pk_u8 pk_cfun_arity_conv(PK_ARITY arity) {
    switch (arity) {
        case PK_ARITY_NORMAL: return PK_INT_ARITY_NORMAL;
        case PK_ARITY_OPTIONAL: return PK_INT_ARITY_OPTIONAL;
        case PK_ARITY_VARIADIC: return PK_INT_ARITY_VARIADIC;
        default: return PK_INT_ARITY_NORMAL;
    }
}

PKAtom pk_cfun0(void *user_closure, PK_CALL call, PKFun0 fun) {
    return pk_cfun_cons(user_closure, call, (void *)fun, 0, PK_INT_ARITY_NORMAL);
}

PKAtom pk_cfun1(void *user_closure, PK_CALL call, PKFun1 fun, PK_ARITY arity) {
    return pk_cfun_cons(user_closure, call, (void *)fun, 1, pk_cfun_arity_conv(arity));
}

PKAtom pk_cfun2(void *user_closure, PK_CALL call, PKFun2 fun, PK_ARITY arity) {
    return pk_cfun_cons(user_closure, call, (void *)fun, 2, pk_cfun_arity_conv(arity));
}

PKAtom pk_cfun3(void *user_closure, PK_CALL call, PKFun3 fun, PK_ARITY arity) {
    return pk_cfun_cons(user_closure, call, (void *)fun, 3, pk_cfun_arity_conv(arity));
}

PKAtom pk_lfun_exec(PKAtom lfun, size_t save) {
    PKVM vm = pk_vm_init(lfun, save);
    return pk_vm_exec(&vm);
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
                    return pk_errors(pk_str("bad bytecode"));
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
                    return pk_errors(pk_str("bad bytecode"));
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

    return pk_string(pk_writer_slice(w));
}
    
PKAtom pk_hexdump(PKAtom lfun) {
    PKAtom a = lfun;
    if (pk_is_symbol(a)) {
        a = pk_get(PK_ENV_FUN, a);
        pk_propagate(a);
    } else {
        pk_propagate(pk_ref_inc(a));
    }
    
    pk_propagate(pk_typecheck_lfun(a));
    
    PKWriter w = PK_WRITER_EMPTY;
    PKAtom result = pk_hexdump_(&w, a);
    pk_writer_deinit(&w);
    pk_ref_dec(a);
    
    return result;
}

bool pk_is_cfun(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_CFUN;
}

bool pk_is_lfun(PKAtom atom) {
    return atom->header.type == PK_ATOMTY_LFUN;
}

bool pk_is_function(PKAtom atom) {
    return pk_is_cfun(atom) || pk_is_lfun(atom);
}

PKAtom pk_typecheck_lfun(PKAtom atom) {
    return pk_typecheck(atom, PK_TYPE_LFUN);
}

PKAtom pk_typecheck_cfun(PKAtom atom) {
    return pk_typecheck(atom, PK_TYPE_CFUN);
}

PKAtom pk_typecheck_function(PKAtom atom) {
    return pk_typecheck(atom, PK_TYPE_FUNCTION);
}
