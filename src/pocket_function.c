#include "pocket_internals.h"

PKAtomCFun *pk_atom_cfun(Pocket lisp, void *user_closure, PKFun fun, PK_CALL call, PK_ARITY arity, pk_u8 args, pk_u8 options) {
    if (arity == PK_ARITY_OPTIONAL) {
        if (options == 0) {
            pk_errorf(lisp, "pk_atom_cfun: optional arity requires at least 1 optional, got 0");
            return NULL;
        }
        if (options > args) {
            pk_errorf(lisp, "pk_atom_cfun: %u options exceeds the %u declared args", options, args);
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

PK_RES pk_atom_cfun_exec(Pocket lisp, PKAtomCFun *cfun) {
    (cfun->fun)(cfun->user_closure, lisp);
    return PK_OK;
}

PK_RES pk_atom_lfun_exec(Pocket lisp, PKAtomLFun *lfun) {
    PKVM vm = pk_vm_init(lisp, (PKAtom *)lfun, 0);
    return pk_vm_exec(lisp, &vm);
}

PKAtomString *pk_lfun_hexdump_(Pocket lisp, PKWriter *w, PKAtomLFun *lfun) {
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
                    pk_errorf(lisp, "pk_lfun_hexdump_: bytecode truncated, operand missing at offset %zu of %zu",
                        i, lfun->bc.length);
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
                    pk_errorf(lisp, "pk_lfun_hexdump_: bytecode truncated, operand missing at offset %zu of %zu",
                        i, lfun->bc.length);
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
    
PKAtomString *pk_lfun_hexdump(Pocket lisp, PKAtomLFun *lfun) {
    PKWriter w = PK_WRITER_EMPTY;
    PKAtomString *result = pk_lfun_hexdump_(lisp, &w, lfun);
    pk_writer_deinit(lisp, &w);
    return result;
}

PKAtomString *pk_atom_hexdump(Pocket lisp, PKAtom *atom) {
    PKAtom *a = atom;
    if (pk_atom_is_symbol(a)) {
        a = pk_env_get(lisp, PK_ENV_FUN, (PKAtomSymbol *)a);
        if (pk_atom_is_undefined(a)) {
            PKStr name = ((PKAtomSymbol *)atom)->name->str;
            pk_errorf(lisp, "pk_atom_hexdump: undefined function %.*s", (int)name.length, name.c);
            return lisp->cache.empty_string;
        }
    }
    PKAtomLFun *lfun = pk_atom_assert_lfun(lisp, a);
    return pk_lfun_hexdump(lisp, lfun);
}

bool pk_atom_is_cfun(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_CFUN;
}

bool pk_atom_is_lfun(PKAtom *atom) {
    return atom->header.type == PK_ATOMTY_LFUN;
}

bool pk_atom_is_function(PKAtom *atom) {
    return pk_atom_is_cfun(atom) || pk_atom_is_lfun(atom);
}

PKAtomLFun *pk_atom_assert_lfun(Pocket lisp, PKAtom *atom) {
    if (!pk_atom_is_lfun(atom)) {
        PKStr ty = pk_ident_atomty(atom->header.type);
        pk_errorf(lisp, "pk_atom_assert_lfun: expected a lfun, got %.*s", (int)ty.length, ty.c);
    }
    return (PKAtomLFun *)atom;
}

PKAtomCFun *pk_atom_assert_cfun(Pocket lisp, PKAtom *atom) {
    if (!pk_atom_is_cfun(atom)) {
        PKStr ty = pk_ident_atomty(atom->header.type);
        pk_errorf(lisp, "pk_atom_assert_cfun: expected a cfun, got %.*s", (int)ty.length, ty.c);
    }
    return (PKAtomCFun *)atom;
}
