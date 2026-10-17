#include "pocket_internals.h"

PKAtomCFun *pk_atom_cfun(Pocket lisp, void *user_closure, PKFun fun, PK_CALL call, PK_ARITY arity, pk_u8 args, pk_u8 options) {
    if (arity == PK_ARITY_OPTIONAL) {
        if (options == 0) {
            pk_error(lisp);
            return NULL;
        }
        if (options > args) {
            pk_error(lisp);
            return NULL;
        }
    }
    
    PKAtomCFun *atom = (PKAtomCFun *)pk_atom_alloc(lisp);

    if (call == PK_CALL_FUN) {
        atom->header.type = PK_ATOMTY_CFUN;
    } else {
        atom->header.type = PK_ATOMTY_CMACRO;
    }
    
    atom->user_closure = user_closure;
    atom->fun = fun;
    atom->arity = (PKArity) {
        .type = (pk_u8)arity,
        .args = args,
        .options = options,
    };
        
    return atom;
}

void pk_atom_cfun_exec(Pocket lisp, PKAtomCFun *cfun) {
    (cfun->fun)(cfun->user_closure, lisp);
}

void pk_atom_lfun_exec(Pocket lisp, PKAtomLFun *lfun) {
    (void)lisp;
    PKVM vm = pk_vm_init((PKAtom *)lfun, 0);
    pk_vm_exec(&vm);
}

PKAtomString *pk_hexdump_(Pocket lisp, PKWriter *w, PKAtomLFun *lfun) {
    pk_writer_string(lisp, w, pk_str("*~HEX-DUMP~*\n"));
    pk_writer_string(lisp, w, pk_str("SECTION-CODE:\n"));
    
    for (size_t i = 0; i < lfun->bc.length; ++i) {
        pk_writer_string(lisp, w, pk_str("["));
        pk_writer_fmt(lisp, w, "%zu", i);
        pk_writer_string(lisp, w, pk_str("] => "));
        
        pk_u8 byte = lfun->bc.e[i];
        PKStr ident = pk_ident_opcode(byte);
        pk_writer_string(lisp, w, ident);
        
        PK_OPCODE_TY ty = pk_opcode_ty(byte);
        
        switch (ty) {
            case PK_OPCODE_TY_LOAD: {
                i++;
                if (i >= lfun->bc.length) {
                    pk_error(lisp);
                    return lisp->cache.empty_string;
                }
                pk_u8 data = lfun->bc.e[i];
                PKAtom *atom = lfun->atoms.e[data];
                pk_writer_string(lisp, w, pk_str(" => "));
                pk_writer_atom(lisp, w, atom);
                break;
            }
            case PK_OPCODE_TY_LIT: {
                i++;
                if (i >= lfun->bc.length) {
                    pk_error(lisp);
                    return lisp->cache.empty_string;
                }
                pk_u8 data = lfun->bc.e[i];
                
                pk_writer_string(lisp, w, pk_str(" => "));
                pk_writer_fmt(lisp, w, "%u", data);
                break;
            }
            default: break;
        }
        pk_writer_newline(lisp, w);
    }

    pk_writer_newline(lisp, w);
    
    pk_writer_string(lisp, w, pk_str("SECTION-DATA:\n"));
    for (size_t i = 0; i < lfun->atoms.length; ++i) {
        pk_writer_atom(lisp, w, lfun->atoms.e[i]);
        pk_writer_newline(lisp, w);
    }
    
    pk_writer_newline(lisp, w);

    return pk_atom_string(lisp, pk_writer_slice(w));
}
    
PKAtom pk_hexdump(PKAtom lfun) {
    PKAtom a = lfun;
    if (pk_is_symbol(a)) {
        a = pk_get(PK_ENV_FUN, a);
        a;
    }
    pk_ref_inc(a);
    
    pk_typecheck_lfun(a);
    
    PKWriter w = PK_WRITER_EMPTY;
    PKAtom result = pk_hexdump_(&w, a);
    pk_writer_deinit(lisp, &w);
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
