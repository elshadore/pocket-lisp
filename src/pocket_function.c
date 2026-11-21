#include "pocket_internals.h"

PKAtomCFun *pk_atom_cfun(Pocket lisp, void *user_closure, PKFun fun, PK_CALL call, PKFunArgs *args) {
    if (args->arity == PK_ARITY_OPTIONAL) {
        if (args->options == 0) {
            pk_errorf(lisp, "pk_atom_cfun: optional arity requires at least 1 optional, got 0");
            return NULL;
        }
        if (args->options > args->args) {
            pk_errorf(lisp, "pk_atom_cfun: %u options exceeds the %u declared args", args->options, args->args);
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
        .type = (pk_u8)args->arity,
        .args = args->args,
        .options = args->options,
    };
    atom->props = PK_HASHMAP_EMPTY;

    return atom;
}

PKAtom *pk_atom_cfun_put(Pocket lisp, PKAtomCFun *cfun, PKAtom *key, PKAtom *value) {
    return pk_atom_property_put(lisp, (PKAtom *)cfun, &cfun->props, key, value);
}

PKAtom *pk_atom_cfun_remove(Pocket lisp, PKAtomCFun *cfun, PKAtom *key) {
    return pk_atom_property_remove(lisp, &cfun->props, key);
}

PKAtom *pk_atom_cfun_get(Pocket lisp, PKAtomCFun *cfun, PKAtom *key) {
    return pk_atom_property_get(lisp, &cfun->props, key);
}

bool pk_atom_cfun_is_property(Pocket lisp, PKAtomCFun *cfun, PKAtom *key) {
    return pk_atom_property_is_property(lisp, &cfun->props, key);
}

PKAtom *pk_atom_cfun_to_plist(Pocket lisp, PKAtomCFun *cfun) {
    return pk_atom_property_to_plist(lisp, &cfun->props);
}

void pk_atom_cfun_exec(Pocket lisp, PKAtomCFun *cfun) {
    (cfun->fun)(cfun->user_closure, lisp);
}

void pk_atom_lfun_exec(Pocket lisp, PKAtomLFun *lfun) {
    PKVM vm = pk_vm_init((PKAtom *)lfun, 0);
    pk_vm_exec(lisp, &vm);
}

PKAtom *pk_atom_lfun_put(Pocket lisp, PKAtomLFun *lfun, PKAtom *key, PKAtom *value) {
    return pk_atom_property_put(lisp, (PKAtom *)lfun, &lfun->props, key, value);
}

PKAtom *pk_atom_lfun_remove(Pocket lisp, PKAtomLFun *lfun, PKAtom *key) {
    return pk_atom_property_remove(lisp, &lfun->props, key);
}

PKAtom *pk_atom_lfun_get(Pocket lisp, PKAtomLFun *lfun, PKAtom *key) {
    return pk_atom_property_get(lisp, &lfun->props, key);
}

bool pk_atom_lfun_is_property(Pocket lisp, PKAtomLFun *lfun, PKAtom *key) {
    return pk_atom_property_is_property(lisp, &lfun->props, key);
}

PKAtom *pk_atom_lfun_to_plist(Pocket lisp, PKAtomLFun *lfun) {
    return pk_atom_property_to_plist(lisp, &lfun->props);
}

PKAtomString *pk_lfun_hexdump(Pocket lisp, PKAtomLFun *lfun) {
    pk_writer_reset(lisp);
    
    pk_writer_string(lisp, pk_str("*~HEX-DUMP~*\n"));
    pk_writer_string(lisp, pk_str("SECTION-CODE:\n"));
    
    for (size_t i = 0; i < lfun->bc.length; ++i) {
        pk_writer_string(lisp, pk_str("["));
        pk_writer_fmt(lisp, "%zu", i);
        pk_writer_string(lisp, pk_str("] => "));
        
        pk_u8 byte = lfun->bc.e[i];
        PKStr ident = pk_ident_opcode(byte);
        pk_writer_string(lisp, ident);
        
        PK_OPCODE_TY ty = pk_opcode_ty(byte);
        
        switch (ty) {
            case PK_OPCODE_TY_LOAD: {
                i++;
                if (i >= lfun->bc.length) {
                    pk_errorf(lisp, "pk_lfun_hexdump_: bytecode truncated, operand missing at offset %zu of %zu",
                        i, lfun->bc.length);
                    return &PK_ATOM_EMPTY_STRING;
                }
                pk_u8 data = lfun->bc.e[i];
                PKAtom *atom = lfun->atoms.e[data];
                pk_writer_string(lisp, pk_str(" => "));
                pk_writer_atom(lisp, atom);
                break;
            }
            case PK_OPCODE_TY_LIT: {
                i++;
                if (i >= lfun->bc.length) {
                    pk_errorf(lisp, "pk_lfun_hexdump_: bytecode truncated, operand missing at offset %zu of %zu",
                        i, lfun->bc.length);
                    return &PK_ATOM_EMPTY_STRING;
                }
                pk_u8 data = lfun->bc.e[i];
                
                pk_writer_string(lisp, pk_str(" => "));
                pk_writer_fmt(lisp, "%u", data);
                break;
            }
            default: break;
        }
        pk_writer_newline(lisp);
    }

    pk_writer_newline(lisp);
    
    pk_writer_string(lisp, pk_str("SECTION-DATA:\n"));
    for (size_t i = 0; i < lfun->atoms.length; ++i) {
        pk_writer_atom(lisp, lfun->atoms.e[i]);
        pk_writer_newline(lisp);
    }
    
    pk_writer_newline(lisp);

    PKAtomString *result = pk_atom_string(lisp, pk_writer_str(lisp));
    pk_writer_reset(lisp);
    return result;
}

PKAtomString *pk_atom_hexdump(Pocket lisp, PKAtom *atom) {
    PKAtom *a = atom;
    if (pk_atom_is_symbol(a)) {
        a = pk_env_fetch(lisp, PK_ENV_FUN, (PKAtomSymbol *)a);
        if (pk_atom_is_undefined(a)) {
            PKStr name = ((PKAtomSymbol *)atom)->name->str;
            pk_errorf(lisp, "pk_atom_hexdump: undefined function %.*s", (int)name.length, name.c);
            return &PK_ATOM_EMPTY_STRING;
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
