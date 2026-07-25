#include "pocket_internals.h"

#define pk_cmp_addr(c_) ((c_)->addr)

#define pk_compiler_error(c_) pk_error((c_)->lisp)

PKCompiler pk_compiler_new(Pocket lisp) {
    PKCompiler c;
    
    c.lisp = lisp;
    c.atoms.e = NULL;
    c.atoms.count = 0;
    c.atoms.capacity = 0;
    c.bc.e = NULL;
    c.bc.count = 0;
    c.bc.capacity = 0;

    c.addr = 0;
    
    return c;
}

PK_RES pk_cmp_patch_byte(PKCompiler *c, pk_u8 addr, pk_u8 byte) {
    if (addr >= c->bc.count) {
        return pk_compiler_error(c);
    }
    c->bc.e[addr] = byte;
    return PK_OK;
}

PK_RES pk_cmp_push_byte(PKCompiler *c, pk_u8 byte) {
    if (c->bc.count >= UCHAR_MAX) {
        return pk_compiler_error(c);
    }
    
    c->addr = (pk_u8)c->bc.count;
    
    return pk_bytes_push(c->lisp, &c->bc, byte, PK_COMPILER_INIT_CAPACITY);
}

PK_RES pk_cmp_push_atom(PKCompiler *c, PKAtom *atom) {
    size_t i = 0;
    
    if (c->atoms.count >= UCHAR_MAX) {
        return pk_compiler_error(c);
    }

    for (i = 0; i < c->atoms.count; ++i) {
        PKAtom *cmp = c->atoms.e[i];
        if (pk_atom_eq(c->lisp, atom, cmp)) {
            c->addr = (pk_u8)i;
            return PK_OK;
        }
    }
    
    c->addr = (pk_u8)c->atoms.count;
    return pk_atoms_push(c->lisp, &c->atoms, atom, PK_COMPILER_INIT_CAPACITY);
}

PK_RES pk_cmp_load_nil(PKCompiler *c) {
    pk_try(pk_cmp_push_byte(c, PK_OP_LOAD_NIL));
    return PK_OK;
}
    
PK_RES pk_cmp_load(PKCompiler *c, PKAtom *atom) {
    pk_u8 addr = 0;

    if (pk_atom_is_nil(atom)) {
        return pk_cmp_load_nil(c);
    }
    
    pk_try(pk_cmp_push_atom(c, atom));
    addr = pk_cmp_addr(c);
    
    pk_try(pk_cmp_push_byte(c, PK_OP_LOAD));
    pk_try(pk_cmp_push_byte(c, addr));
    
    return PK_OK;
}

PK_RES pk_cmp_lookup(PKCompiler *c, PKAtomSymbol *symbol, PKEnvTy env) {
    pk_u8 addr = 0;
    
    pk_try(pk_cmp_push_atom(c, (PKAtom *)symbol));
    addr = pk_cmp_addr(c);

    switch (env) {
        case PKEnvTy_Var: {
            pk_try(pk_cmp_push_byte(c, PK_OP_LOOKUP_VAR));
            break;
        }
        case PKEnvTy_Fun: {
            pk_try(pk_cmp_push_byte(c, PK_OP_LOOKUP_FUN));
            break;
        }
    }
    
    pk_try(pk_cmp_push_byte(c, addr));
    
    return PK_OK;
}

PK_RES pk_compile_evlist(PKCompiler *c, PKAtom *args) {
    PKAtom *iter = args;
    
    while (!pk_atom_is_nil(iter)) {
        PKAtomCons *cons = NULL;
        pk_try(pk_atom_cast_cons(c->lisp, iter, &cons));

        pk_try(pk_compile_value(c, cons->car));

        iter = cons->cdr;
    }

    return PK_OK;
}

PK_RES pk_compile_let(PKCompiler *c, PKAtom *args, PKEnvTy env) {
    PKAtomCons *cons = NULL;
    PKAtom *bindings = NULL;
    PKAtom *body = NULL;
    PKAtom *iter = NULL;
    size_t lets = 0;
    
    pk_try(pk_atom_cast_cons(c->lisp, args, &cons));
    bindings = cons->car;
    body = cons->cdr;
    
    pk_try(pk_cmp_push_byte(c, PK_OP_BLOCK_BEGIN));

    iter = bindings;
    while (!pk_atom_is_nil(iter)) {
        PKAtomCons *ca = NULL;
        
        pk_try(pk_atom_cast_cons(c->lisp, iter, &ca));
        
        if (pk_atom_is_symbol(ca->car)) {
            pk_try(pk_cmp_load_nil(c));
        } else {
            PKAtomCons *cb = NULL;
            PKAtomCons *cc = NULL;
            
            pk_try(pk_atom_cast_cons(c->lisp, ca->car, &cb));
            pk_try(pk_atom_cast_cons(c->lisp, cb->cdr, &cc));

            pk_try(pk_atom_assert_nil(c->lisp, cc->cdr));

            pk_try(pk_compile_value(c, cc->car));
        }

        lets += 1;
        iter = ca->cdr;
    }
    
    iter = bindings;
    while (!pk_atom_is_nil(iter)) {
        PKAtomCons *ca = NULL;
        PKAtomSymbol *symbol = NULL;
        
        pk_try(pk_atom_cast_cons(c->lisp, iter, &ca));
        
        if (pk_atom_is_symbol(ca->car)) {
            symbol = (PKAtomSymbol *)ca->car;
        } else {
            PKAtomCons *cb = NULL;
            pk_try(pk_atom_cast_cons(c->lisp, ca->car, &cb));
            pk_try(pk_atom_cast_symbol(c->lisp, cb->car, &symbol));
        }
        pk_try(pk_cmp_load(c, (PKAtom *)symbol));
        
        iter = ca->cdr;
    }

    switch (env) {
        case PKEnvTy_Var: {
            pk_try(pk_cmp_push_byte(c, PK_OP_LET_VAR));
            break;
        }
        case PKEnvTy_Fun: {
            pk_try(pk_cmp_push_byte(c, PK_OP_LET_FUN));
            break;
        }
    }
    
    pk_try(pk_cmp_push_byte(c, (pk_u8)lets));
    pk_try(pk_compile_evlist(c, body));
    pk_try(pk_cmp_push_byte(c, PK_OP_BLOCK_END));
    return PK_OK;
}

PK_RES pk_compile_special(PKCompiler *c, PKAtomSymbol *symbol, PKAtom *args, pk_bool *is_special) {
    if (symbol == c->lisp->cache.lambda) {
        PKAtomCons *cons = NULL;
        PKAtom *lambda_args = NULL;
        PKAtom *body = NULL;
        PKAtomLFunc *lfunc = NULL;
        
        pk_try(pk_atom_cast_cons(c->lisp, args, &cons));
        lambda_args = cons->car;
        body = cons->cdr;

        pk_try(pk_compile_lambda(c->lisp, lambda_args, body, &lfunc));
        pk_try(pk_cmp_load(c, (PKAtom *)lfunc));
        
        *is_special = PK_TRUE;
    } else if (symbol == c->lisp->cache.quote) {
        PKAtomCons *cons = NULL;
        PKAtom *value = NULL;
        
        pk_try(pk_atom_cast_cons(c->lisp, args, &cons));
        value = cons->car;
        pk_try(pk_atom_assert_nil(c->lisp, cons->cdr));
        pk_try(pk_cmp_load(c, value));
        
        *is_special = PK_TRUE;
    } else if (symbol == c->lisp->cache.while_sym) {
        PKAtomCons *cons = NULL;
        PKAtom *predicate = NULL;
        PKAtom *body = NULL;
        pk_u8 start = 0;
        pk_u8 patch = 0;
        
        pk_try(pk_atom_cast_cons(c->lisp, args, &cons));
        predicate = cons->car;
        body = cons->cdr;

        start = pk_cmp_addr(c);
        pk_try(pk_compile_value(c, predicate));
        pk_try(pk_cmp_push_byte(c, PK_OP_JMP_IF_NIL));
        pk_try(pk_cmp_push_byte(c, 0));
        patch = pk_cmp_addr(c);
        
        pk_try(pk_compile_evlist(c, body));
        pk_try(pk_cmp_push_byte(c, PK_OP_JMP_BACK));
        pk_try(pk_cmp_push_byte(c, pk_cmp_addr(c) - start + 1));
        pk_try(pk_cmp_patch_byte(c, patch, pk_cmp_addr(c) + 1 - patch));
        
        *is_special = PK_TRUE;
    } else if (symbol == c->lisp->cache.if_sym) {
        PKAtomCons *cons = NULL;
        PKAtom *value = NULL;
        PKAtom *clause_t = NULL;
        PKAtom *clause_f = NULL;
        pk_u8 jump_to_t = 0;
        pk_u8 jump_from_f = 0;
        pk_u8 patch_1 = 0;
        pk_u8 patch_2 = 0;
        
        pk_try(pk_atom_cast_cons(c->lisp, args, &cons));
        value = cons->car;
        pk_try(pk_atom_cast_cons(c->lisp, cons->cdr, &cons));
        clause_t = cons->car;
        pk_try(pk_atom_cast_cons(c->lisp, cons->cdr, &cons));
        clause_f = cons->car;
        pk_try(pk_atom_assert_nil(c->lisp, cons->cdr));

        pk_try(pk_compile_value(c, value));

        pk_try(pk_cmp_push_byte(c, PK_OP_JMP_IF_NIL));
        pk_try(pk_cmp_push_byte(c, 0));
        patch_1 = pk_cmp_addr(c);
        
        pk_try(pk_compile_value(c, clause_f));
        pk_try(pk_cmp_push_byte(c, PK_OP_JMP));
        pk_try(pk_cmp_push_byte(c, 0));
        patch_2 = pk_cmp_addr(c);
        
        jump_to_t = pk_cmp_addr(c) + 1 - patch_1;
        pk_try(pk_compile_value(c, clause_t));
        jump_from_f = pk_cmp_addr(c) + 1 - patch_2;

        pk_try(pk_cmp_patch_byte(c, patch_1, jump_to_t));
        pk_try(pk_cmp_patch_byte(c, patch_2, jump_from_f));
        
        *is_special = PK_TRUE;
    } else if (symbol == c->lisp->cache.let_sym) {
        pk_try(pk_compile_let(c, args, PKEnvTy_Var));
        
        *is_special = PK_TRUE;
    } else if (symbol == c->lisp->cache.flet_sym) {
        pk_try(pk_compile_let(c, args, PKEnvTy_Fun));
        
        *is_special = PK_TRUE;
    } else if (symbol == c->lisp->cache.progn) {
        pk_try(pk_cmp_push_byte(c, PK_OP_BLOCK_BEGIN));
        pk_try(pk_compile_evlist(c, args));
        pk_try(pk_cmp_push_byte(c, PK_OP_BLOCK_END));
        
        *is_special = PK_TRUE;
    } else {
        *is_special = PK_FALSE;
    }
    return PK_OK;
}

PK_RES pk_compile_expression(PKCompiler *c, PKAtomCons *expr) {
    PKAtom *form = expr->car;
    PKAtom *iter = expr->cdr;
    PKAtomSymbol *symbol = NULL;
    size_t acc = 0;
    pk_bool is_special = PK_FALSE;

    pk_try(pk_atom_cast_symbol(c->lisp, form, &symbol));
    
    pk_try(pk_compile_special(c, symbol, iter, &is_special));

    if (is_special) {
        return PK_OK;
    }
    
    pk_try(pk_cmp_lookup(c, symbol, PKEnvTy_Fun));
    
    while (!pk_atom_is_nil(iter)) {
        PKAtomCons *cons = NULL;
        pk_try(pk_atom_cast_cons(c->lisp, iter, &cons));
        pk_try(pk_compile_value(c, cons->car));
           
        acc += 1;
        iter = cons->cdr;
    }

    if (acc >= UCHAR_MAX) {
        return pk_compiler_error(c);
    }
    

    pk_try(pk_cmp_push_byte(c, PK_OP_CALL));
    pk_try(pk_cmp_push_byte(c, (pk_u8)acc));
    
    return PK_OK;
}

PK_RES pk_compile_value(PKCompiler *c, PKAtom *value) {
    switch (value->tag.ty) {
        case PKAtomTy_Cons: {
            return pk_compile_expression(c, (PKAtomCons *)value);
        }
        case PKAtomTy_Symbol: {
            return pk_cmp_lookup(c, (PKAtomSymbol *)value, PKEnvTy_Var);
        }
        default: {
            return pk_cmp_load(c, value);
        }
    }
}

PK_RES pk_compile_compile(PKCompiler *c, size_t arity, PKAtomLFunc **output) {
    PKAtom *a = NULL;
    size_t i = 0;
    PK_RES result = PK_YIELD;
    pk_defer(pk_atom_alloc(c->lisp, &a));
    
    for (i = c->atoms.count; i < c->atoms.capacity; ++i) {
        c->atoms.e[i] = pk_atom_nil(c->lisp);
    }
    
    for (i = c->bc.count; i < c->bc.capacity; ++i) {
        c->bc.e[i] = PK_OP_ILLEGAL;
    }
    
    a->tag.ty = PKAtomTy_LFunc;
    a->lfunc.atoms.e = c->atoms.e;
    a->lfunc.atoms.length = c->atoms.capacity;
    a->lfunc.bc.e = c->bc.e;
    a->lfunc.bc.length = c->bc.capacity;
    a->lfunc.arity.args = (int)arity;
    a->lfunc.arity.mode = PKArity_Normal;

    *output = (PKAtomLFunc *)a;
    pk_defer(pk_dump_hex_atom(c->lisp, *output));
    
    result = PK_OK;
    
    DEFER:
    if (result == PK_YIELD) {
        pk_atoms_free(c->lisp, &c->atoms);
        pk_bytes_free(c->lisp, &c->bc);
    }
   
    return result;
}

PK_RES pk_compile_lambda(Pocket lisp, PKAtom *args, PKAtom *body, PKAtomLFunc **output) {
    PKCompiler c;
    size_t arity = 0;
    PKAtom *iter = args;

    c = pk_compiler_new(lisp);
    
    while (!pk_atom_is_nil(iter)) {
        PKAtomCons *cons = NULL;
        PKAtomSymbol *symbol = NULL;
        
        pk_try(pk_atom_cast_cons(lisp, iter, &cons));
        pk_try(pk_atom_cast_symbol(lisp, cons->car, &symbol));
        pk_try(pk_cmp_load(&c, (PKAtom *)symbol));

        arity += 1;
        
        iter = cons->cdr;
    }

    if (arity > 0) {
        pk_try(pk_cmp_push_byte(&c, PK_OP_LET_VAR));
        pk_try(pk_cmp_push_byte(&c, (pk_u8)arity));
    }
    
    pk_try(pk_compile_evlist(&c, body));
    pk_try(pk_cmp_push_byte(&c, PK_OP_RET));
    
    pk_try(pk_compile_compile(&c, arity, output));
    
    return PK_OK;
}

PK_RES pk_compile_atom(Pocket lisp, PKAtom *value, PKAtomLFunc **output) {
    PKCompiler c;

    c = pk_compiler_new(lisp);

    pk_try(pk_compile_value(&c, value));
    pk_try(pk_cmp_push_byte(&c, PK_OP_RET));
    
    pk_try(pk_compile_compile(&c, 0, output));

    return PK_OK;
}
