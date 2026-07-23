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

    return c;
}

PKRes pk_cmp_push_byte(PKCompiler *c, pk_u8 byte) {
    if (c->bc.count >= UCHAR_MAX) {
        return pk_compiler_error(c);
    }
    
    c->addr = (pk_u8)c->bc.count;
    
    return pk_bytes_push(c->lisp, &c->bc, byte, PK_COMPILER_INIT_CAPACITY);
}

PKRes pk_cmp_push_atom(PKCompiler *c, PKAtom *atom) {
    if (c->atoms.count >= UCHAR_MAX) {
        return pk_compiler_error(c);
    }
    
    c->addr = (pk_u8)c->atoms.count;
    
    return pk_atoms_push(c->lisp, &c->atoms, atom, PK_COMPILER_INIT_CAPACITY);
}

PKRes pk_cmp_load(PKCompiler *c, PKAtom *atom) {
    pk_u8 addr = 0;
    
    pk_try(pk_cmp_push_atom(c, atom));
    addr = pk_cmp_addr(c);
    
    pk_try(pk_cmp_push_byte(c, PK_OP_LOAD));
    pk_try(pk_cmp_push_byte(c, addr));
    
    return PK_Ok;
}

PKRes pk_compile_evlist(PKCompiler *c, PKAtom *args) {
    PKAtom *iter = args;
    
    while (pk_atom_is_nil(iter)) {
        PKAtomCons *cons = NULL;
        pk_try(pk_atom_cast_cons(c->lisp, iter, &cons));

        pk_try(pk_compile_value(c, cons->car));

        iter = cons->cdr;
    }

    return PK_Ok;
}

PKRes pk_compile_special(PKCompiler *c, PKAtomSymbol *symbol, PKAtom *args, pk_bool *is_special) {
    if (symbol == c->lisp->cache.lambda) {
        *is_special = PK_TRUE;
        return pk_compiler_error(c);
    } else if (symbol == c->lisp->cache.progn) {
        *is_special = PK_TRUE;
        pk_try(pk_cmp_push_byte(c, PK_OP_BLOCK));
        pk_try(pk_compile_evlist(c, args));
        pk_try(pk_cmp_push_byte(c, PK_OP_RET));
    } else {
        *is_special = PK_FALSE;
    }
    return PK_Ok;
}

PKRes pk_compile_expression(PKCompiler *c, PKAtomCons *expr) {
    PKAtom *form = expr->car;
    PKAtom *iter = expr->cdr;
    PKAtomSymbol *symbol = NULL;
    size_t acc = 0;
    pk_bool is_special = PK_FALSE;


    pk_try(pk_atom_cast_symbol(c->lisp, form, &symbol));
    
    pk_try(pk_compile_special(c, symbol, iter, &is_special));

    if (is_special) {
        return PK_Ok;
    }
    
    pk_try(pk_compile_value(c, form));
    
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
    
    return PK_Ok;
}

PKRes pk_compile_value(PKCompiler *c, PKAtom *value) {
    switch (value->tag.ty) {
        case PKAtomTy_Cons: {
            return pk_compile_expression(c, (PKAtomCons *)value);
        }
        default: {
            return pk_cmp_load(c, value);
        }
    }
}

PKRes pk_compile_compile(PKCompiler *c, PKAtomLFunc **output) {
    PKAtom *a = NULL;
    size_t i = 0;
    
    pk_try(pk_atom_alloc(c->lisp, &a));

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

    *output = (PKAtomLFunc *)a;
    return PK_Ok;
}

PKRes pk_compile_atom(Pocket lisp, PKAtom *value, PKAtomLFunc **output) {
    PKCompiler c;

    c = pk_compiler_new(lisp);

    pk_try(pk_compile_value(&c, value));
    pk_try(pk_cmp_push_byte(&c, PK_OP_RET));
    
    if (!pk_compile_compile(&c, output)) {
        pk_atoms_free(lisp, &c.atoms);
        pk_bytes_free(lisp, &c.bc);
        return PK_Yield;
    }
    
    return PK_Ok;
}
