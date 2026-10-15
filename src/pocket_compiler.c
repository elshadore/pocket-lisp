#include "pocket_internals.h"

#define PK_QQ_EMPTY (PKQQ){ \
    .count = 0, \
    .spliced_count = 0, \
    .spliced = false, \
    .spliced_last = false, \
    .packed_cons = false, \
}

typedef struct PKQQ_ {
    size_t count;
    size_t spliced_count;
    bool spliced;
    bool spliced_last;
    bool packed_cons;
} PKQQ;

PKCompiler pk_cmp_new(PK_CALL type) {
    return (PKCompiler) {
        .atoms = PK_ARRAY_EMPTY,
        .bc = PK_BYTES_EMPTY,
        .type = type,
        .addr = 0,
        .protect_depth = 0,
    };
}

#define pk_cmp_addr(c_) ((c_)->addr)

PKAtom pk_cmp_patch_byte(PKCompiler *c, pk_u8 addr, pk_u8 byte) {
    if (addr >= c->bc.count) {
        return pk_errors(pk_str("bad byte address"));
    }
    c->bc.e[addr] = byte;
    return pk_true();
}

PKAtom pk_cmp_push_byte(PKCompiler *c, pk_u8 byte) {
    if (c->bc.count >= UINT8_MAX) {
        return pk_errors(pk_str("bytecode too big"));
    }
    
    c->addr = (pk_u8)c->bc.count;
    
    return pk_bytes_push(&c->bc, byte, 8);
}

PKAtom pk_cmp_push_any(PKCompiler *c, size_t value) {
    if (value > UINT8_MAX) {
        return pk_errors(pk_str("address too big"));
    }

    return pk_cmp_push_byte(c, (pk_u8)value);
}

PKAtom pk_cmp_push_atom(PKCompiler *c, PKAtom atom) {
    if (c->atoms.count >= UINT8_MAX) {
        return pk_errors(pk_str("too many atoms"));
    }

    for (size_t i = 0; i < c->atoms.count; ++i) {
        PKAtom cmp = c->atoms.e[i];
        if (pk_eq(atom, cmp)) {
            c->addr = (pk_u8)i;
            return pk_true();
        }
    }
    
    c->addr = (pk_u8)c->atoms.count;
    pk_propagate(pk_array_push(&c->atoms, atom, 8));
    return pk_ref_inc(atom);
}

PKAtom pk_cmp_load_nil(PKCompiler *c) {
    return pk_cmp_push_byte(c, PK_OP_LOAD_NIL);
}

PKAtom pk_cmp_load_true(PKCompiler *c) {
    return pk_cmp_push_byte(c, PK_OP_LOAD_TRUE);
}
    
PKAtom pk_cmp_load(PKCompiler *c, PKAtom atom) {
    if (pk_is_nil(atom)) {
        return pk_cmp_load_nil(c);
    }
    if (atom->header.type == PK_ATOMTY_TRUE) {
        return pk_cmp_load_true(c);
    }
    
    pk_propagate(pk_cmp_push_atom(c, atom));
    pk_u8 addr = pk_cmp_addr(c);
    
    pk_propagate(pk_cmp_push_byte(c, PK_OP_LOAD));
    pk_propagate(pk_cmp_push_byte(c, addr));
    
    
    return pk_true();
}

PKAtom pk_cmp_lookup(PKCompiler *c, PKAtom symbol, PK_ENV env) {
    pk_propagate(pk_cmp_push_atom(c, symbol));
    pk_u8 addr = pk_cmp_addr(c);

    switch (env) {
        case PK_ENV_VAR: {
            pk_propagate(pk_cmp_push_byte(c, PK_OP_GET_VAR));
            break;
        }
        case PK_ENV_FUN: {
            pk_propagate(pk_cmp_push_byte(c, PK_OP_GET_FUN));
            break;
        }
        case PK_ENV_PROP: {
            pk_propagate(pk_cmp_push_byte(c, PK_OP_GET_PROP));
            break;
        }
    }
    
    pk_propagate(pk_cmp_push_byte(c, addr));
    
    return pk_true();
}

void pk_cmp_deinit(PKCompiler *c) {
    pk_array_deinit(&c->atoms);
    pk_free(c->bc.e);
    *c = pk_cmp_new(c->type);
}

PKAtom pk_compile_evlist(PKCompiler *c, PKAtom args) {
    PKAtom iter = args;
    
    while (pk_is_cons(iter)) {
        pk_propagate(pk_compile_value(c, iter->cons.car));
        iter = iter->cons.cdr;
    }

    return pk_true();
}

pk_u8 pk_pattern_car_op(bool optional) {
    if (optional) {
        return PK_OP_CAR;
    } else {
        return PK_OP_CAR_STRICT;
    }
}

pk_u8 pk_pattern_cdr_op(bool optional) {
    if (optional) {
        return PK_OP_CDR;
    } else {
        return PK_OP_CDR_STRICT;
    }
}

#define PK_PATTERN_PATH_CDR (0)
#define PK_PATTERN_PATH_CAR (1)
#define PK_PATTERN_PATH_MAX (64)

PKAtom pk_compile_pattern_path(PKCompiler *c, pk_u8 *path, size_t length, bool optional) {
    for (size_t i = 0; i < length; ++i) {
        if (path[i] == PK_PATTERN_PATH_CAR) {
            pk_propagate(pk_cmp_push_byte(c, pk_pattern_car_op(optional)));
        } else {
            pk_propagate(pk_cmp_push_byte(c, pk_pattern_cdr_op(optional)));
        }
    }
    return pk_true();
}

PKAtom pk_compile_pattern_cdrs(PKCompiler *c, size_t index, bool optional) {
    pk_u8 op = pk_pattern_cdr_op(optional);
    for (size_t i = 0; i < index; ++i) {
        pk_propagate(pk_cmp_push_byte(c, op));
    }
    return pk_true();
}

PKAtom pk_compile_pattern_descend(pk_u8 *path, size_t length, size_t index) {
    if ((length + index + 1) >= PK_PATTERN_PATH_MAX) {
        return pk_errors(pk_str("destructure too deep"));
    }
    
    for (size_t i = 0; i < index; ++i) {
        path[length + i] = PK_PATTERN_PATH_CDR;
    }
    path[length + index] = PK_PATTERN_PATH_CAR;
    
    return pk_true();
}

PKAtom pk_compile_pattern_values_(PKCompiler *c, PKAtom pattern, pk_u8 *path, size_t length, size_t base, size_t bias, size_t *pushed, bool optional) {
    PKAtom iter = pattern;
    size_t index = 0;
    while (pk_is_cons(iter)) {
        PKAtom atom = iter->cons.car;
        PKAtom tail = iter->cons.cdr;

        if (pk_is_keyword(atom)) {
            if ((pk_keyword_qeq(atom, pk_str("opt"))) || (pk_keyword_qeq(atom, pk_str("optional")))) {
                optional = true;
            } else if (pk_keyword_qeq(atom, pk_str("rest"))) {
                if ((!pk_is_cons(tail)) || (!pk_is_symbol(tail->cons.car)) || (!pk_is_nil(tail->cons.cdr))) {
                    return pk_errors(pk_str("bad destructure"));
                }

                pk_propagate(pk_cmp_push_byte(c, PK_OP_DIG));
                pk_propagate(pk_cmp_push_any(c, (*pushed - base) + bias));
                pk_propagate(pk_compile_pattern_path(c, path, length, optional));
                pk_propagate(pk_compile_pattern_cdrs(c, index, true));
                *pushed += 1;
                return pk_true();
            } else {
                return pk_errors(pk_str("bad destructure"));
            }
        } else if (pk_is_symbol(atom)) {
            pk_propagate(pk_cmp_push_byte(c, PK_OP_DIG));
            pk_propagate(pk_cmp_push_any(c, (*pushed - base) + bias));
            pk_propagate(pk_compile_pattern_path(c, path, length, optional));
            pk_propagate(pk_compile_pattern_cdrs(c, index, optional));
            pk_propagate(pk_cmp_push_byte(c, pk_pattern_car_op(optional)));
            *pushed += 1;
        } else if (pk_is_cons(atom)) {
            pk_propagate(pk_compile_pattern_descend(path, length, index));
            pk_propagate(pk_compile_pattern_values_(c, atom, path, length + index + 1, base, bias, pushed, optional));
        } else {
            return pk_errors(pk_str("bad destructure"));
        }

        iter = tail;
        if (!pk_is_keyword(atom)) {
            index += 1;
        }
    }

    if (!pk_is_nil(iter)) {
        return pk_errors(pk_str("bad destructure"));
    }

    return pk_true();
}

PKAtom pk_compile_pattern_values(PKCompiler *c, PKAtom pattern, size_t bias, bool optional, size_t *pushed) {
    pk_u8 path[PK_PATTERN_PATH_MAX] = {0};
    return pk_compile_pattern_values_(c, pattern, path, 0, 0, bias, pushed, optional);
}

PKAtom pk_compile_pattern_symbols_(PKCompiler *c, PKAtom pattern, bool optional, size_t *leaves) {
    PKAtom iter = pattern;
    while (pk_is_cons(iter)) {
        PKAtom atom = iter->cons.car;
        PKAtom tail = iter->cons.cdr;

        if (pk_is_keyword(atom)) {
            if ((pk_keyword_qeq(atom, pk_str("opt"))) || (pk_keyword_qeq(atom, pk_str("optional")))) {
                optional = true;
            } else if (pk_keyword_qeq(atom, pk_str("rest"))) {
                if ((!pk_is_cons(tail)) || (!pk_is_symbol(tail->cons.car)) || (!pk_is_nil(tail->cons.cdr))) {
                    return pk_errors(pk_str("bad destructure"));
                }
                pk_propagate(pk_cmp_load(c, tail->cons.car));
                *leaves += 1;
                return pk_true();
            } else {
                return pk_errors(pk_str("bad destructure"));
            }
        } else if (pk_is_symbol(atom)) {
            pk_propagate(pk_cmp_load(c, atom));
            *leaves += 1;
        } else if (pk_is_cons(atom)) {
            pk_propagate(pk_compile_pattern_symbols_(c, atom, optional, leaves));
        } else {
            return pk_errors(pk_str("bad destructure"));
        }

        iter = tail;
    }

    if (!pk_is_nil(iter)) {
        return pk_errors(pk_str("bad destructure"));
    }

    return pk_true();
}

PKAtom pk_compile_pattern_symbols(PKCompiler *c, PKAtom pattern, bool optional, size_t *leaves) {
    return pk_compile_pattern_symbols_(c, pattern, optional, leaves);
}

bool pk_let_binding_is_pattern(PKAtom binding) {
    return pk_is_cons(binding) && pk_is_cons(binding->cons.car);
}

bool pk_compile_let_has_patterns(PKAtom bindings) {
    PKAtom iter = bindings;
    while (pk_is_cons(iter)) {
        if (pk_let_binding_is_pattern(iter->cons.car)) {
            return true;
        }
        iter = iter->cons.cdr;
    }
    return false;
}

PKAtom pk_compile_let_lets(PKCompiler *c, PK_ENV env) {
    switch (env) {
        case PK_ENV_VAR: {
            pk_propagate(pk_cmp_push_byte(c, PK_OP_LET_VAR));
            break;
        }
        case PK_ENV_FUN: {
            pk_propagate(pk_cmp_push_byte(c, PK_OP_LET_FUN));
            break;
        }
        case PK_ENV_PROP: {
            pk_propagate(pk_cmp_push_byte(c, PK_OP_LET_PROP));
            break;
        }
    }
    return pk_true();
}

PKAtom pk_compile_let_binding_value(PKCompiler *c, PKAtom binding) {
    if (pk_is_symbol(binding)) {
        return pk_cmp_load_nil(c);
    }

    PKAtom a = binding;
    pk_propagate(pk_typecheck_cons(a));
    PKAtom b = a->cons.cdr;
    pk_propagate(pk_typecheck_cons(b));
    if (!pk_is_nil(b->cons.cdr)) {
        return pk_errors(pk_str("bad body"));
    }
    return pk_compile_value(c, b->cons.car);
}

PKAtom pk_compile_let(PKCompiler *c, PKAtom args, PK_ENV env) {
    pk_propagate(pk_typecheck_cons(args));
    
    PKAtom bindings = args->cons.car;
    PKAtom body = args->cons.cdr;
    
    pk_propagate(pk_cmp_push_byte(c, PK_OP_BLOCK_BEGIN));

    bool patterns = pk_compile_let_has_patterns(bindings);

    size_t values = 0;
    {
        PKAtom iter = bindings;
        while (pk_is_cons(iter)) {
            pk_propagate(pk_compile_let_binding_value(c, iter->cons.car));
            values += 1;
            iter = iter->cons.cdr;
        }
    }

    if (patterns) {
        size_t pushed = 0;
        size_t index = 0;
        PKAtom iter = bindings;
        while (pk_is_cons(iter)) {
            PKAtom binding = iter->cons.car;
            if (pk_let_binding_is_pattern(binding)) {
                pk_propagate(pk_compile_pattern_values(c, binding->cons.car, (values - 1) - index, false, &pushed));
            } else {
                pk_propagate(pk_cmp_push_byte(c, PK_OP_DIG));
                pk_propagate(pk_cmp_push_any(c, pushed + ((values - 1) - index)));
                pushed += 1;
            }
            index += 1;
            iter = iter->cons.cdr;
        }
    }

    size_t lets = 0;
    {
        PKAtom iter = bindings;
        while (pk_is_cons(iter)) {
            PKAtom binding = iter->cons.car;
            if (pk_let_binding_is_pattern(binding)) {
                pk_propagate(pk_compile_pattern_symbols(c, binding->cons.car, false, &lets));
            } else {
                PKAtom symbol = pk_nil();
                if (pk_is_symbol(binding)) {
                    symbol = binding;
                } else {
                    pk_propagate(pk_typecheck_cons(binding));
                    pk_propagate(pk_typecheck_symbol(binding->cons.car));
                    symbol = binding->cons.car;
                }
                pk_propagate(pk_cmp_load(c, symbol));
                lets += 1;
            }

            iter = iter->cons.cdr;
        }
    }

    pk_propagate(pk_compile_let_lets(c, env));
    pk_propagate(pk_cmp_push_any(c, lets));
    pk_propagate(pk_compile_evlist(c, body));
    pk_propagate(pk_cmp_push_byte(c, PK_OP_BLOCK_END));
    return pk_true();
}

PKAtom pk_compile_let_star(PKCompiler *c, PKAtom args, PK_ENV env) {
    if (!pk_is_cons(args)) {
        return pk_errors(pk_str("bad let args"));
    }
    
    PKAtom bindings = args->cons.car;
    PKAtom body = args->cons.cdr;
    
    pk_propagate(pk_cmp_push_byte(c, PK_OP_BLOCK_BEGIN));

    PKAtom iter = bindings;
    while (pk_is_cons(iter)) {
        PKAtom binding = iter->cons.car;

        if (pk_let_binding_is_pattern(binding)) {
            PKAtom a = binding->cons.car;
            PKAtom b = binding->cons.cdr;
            pk_propagate(pk_typecheck_cons(b));
            if (!pk_is_nil(b->cons.cdr)) {
                return pk_errors(pk_str("bad let binding"));
            }

            pk_propagate(pk_compile_value(c, b->cons.car));

            size_t pushed = 0;
            pk_propagate(pk_compile_pattern_values(c, a, 0, false, &pushed));

            size_t lets = 0;
            pk_propagate(pk_compile_pattern_symbols(c, a, false, &lets));

            pk_propagate(pk_compile_let_lets(c, env));
            pk_propagate(pk_cmp_push_any(c, lets));

            iter = iter->cons.cdr;
            continue;
        }

        PKAtom symbol = pk_nil();
        if (pk_is_symbol(binding)) {
            pk_propagate(pk_cmp_load_nil(c));
            symbol = binding;
        } else {
            PKAtom a = binding;
            if (!pk_is_cons(a)) {
                return pk_errors(pk_str("bad let binding"));
            }
            PKAtom b = a->cons.cdr;
            if (!pk_is_cons(b)) {
                return pk_errors(pk_str("bad let binding"));
            }
            if (!pk_is_nil(b->cons.cdr)) {
                return pk_errors(pk_str("bad let binding"));
            }
            if (!pk_is_symbol(a->cons.car)) {
                return pk_errors(pk_str("bad let symbol"));
            }
            
            pk_propagate(pk_compile_value(c, b->cons.car));
            symbol = a->cons.car;
        }

        pk_propagate(pk_cmp_load(c, symbol));
        pk_propagate(pk_compile_let_lets(c, env));
        pk_propagate(pk_cmp_push_byte(c, 1));
        
        iter = iter->cons.cdr;
    }
    
    pk_propagate(pk_compile_evlist(c, body));
    pk_propagate(pk_cmp_push_byte(c, PK_OP_BLOCK_END));
    
    return pk_true();
}

PKAtom pk_compile_quasiquote(PKCompiler *c, PKAtom value);
PKAtom pk_compile_quasiquote_list(PKCompiler *c, PKAtom list);
PKAtom pk_compile_quasiquote_value(PKCompiler *c, PKAtom value, PKAtom *spliced);

PKAtom pk_compile_quasiquote_value(PKCompiler *c, PKAtom value, PKAtom *spliced) {
    if (!pk_is_cons(value)) {
        return pk_cmp_load(c, value);
    }
    
    PKAtom a = value->cons.car;
    
    if (pk_is_symbol(a)) {
        if (a == PK_CORE.cache.unquote) {
            PKAtom b = value->cons.cdr;
            if (!pk_is_cons(b)) {
                return pk_errors(pk_str("bad unquote"));
            }
            if (!pk_is_nil(b->cons.cdr)) {
                return pk_errors(pk_str("bad unquote"));
            }
            pk_propagate(pk_compile_value(c, b->cons.car));
            return pk_true();
        } else if (a == PK_CORE.cache.unquote_splice) {
            PKAtom b = value->cons.cdr;
            if (!pk_is_cons(b)) {
                return pk_errors(pk_str("bad unquote splice"));
            }
            if (!pk_is_nil(b->cons.cdr)) {
                return pk_errors(pk_str("bad unquote splice"));
            }
            *spliced = b->cons.car;
            return pk_true();
        }
    } 
    return pk_compile_quasiquote_list(c, value);
}

PKAtom pk_compile_qqlist(PKCompiler *c, PKQQ *qq) {
    if (qq->packed_cons) {
        pk_propagate(pk_cmp_push_byte(c, PK_OP_MAKE_LIST_PACKED));
    } else {
        pk_propagate(pk_cmp_push_byte(c, PK_OP_MAKE_LIST));
    }
    pk_propagate(pk_cmp_push_any(c, qq->count));
    
    qq->packed_cons = false;
    qq->count = 0;
    qq->spliced_count += 1;

    return pk_true();
}

PKAtom pk_compile_quasiquote_list(PKCompiler *c, PKAtom list) {
    PKQQ qq = PK_QQ_EMPTY;

    PKAtom iter = list;
    
    while (!pk_is_nil(iter)) {
        
        PKAtom spliced = NULL;
        
        if (pk_is_cons(iter)) {
            pk_propagate(pk_compile_quasiquote_value(c, iter->cons.car, &spliced));
            iter = iter->cons.cdr;
        } else {
            pk_propagate(pk_compile_quasiquote_value(c, iter, &spliced));
            qq.packed_cons = true;
            iter = pk_nil();
        }
        
        if (spliced != NULL) {
            qq.spliced = true;
            qq.spliced_last = true;
            pk_propagate(pk_compile_qqlist(c, &qq));
            pk_propagate(pk_compile_value(c, spliced));
        } else {
            qq.spliced_last = false;
            qq.count += 1;
        }
    }

    if (!qq.spliced_last) {
        pk_propagate(pk_compile_qqlist(c, &qq));
    }
    
    if (qq.spliced) {
        pk_propagate(pk_cmp_push_byte(c, PK_OP_MERGE_LISTS));
        pk_propagate(pk_cmp_push_any(c, qq.spliced_count + 1));
    }
    
    return pk_true();
}

PKAtom pk_cons_last(PKAtom list) {
    PKAtom last = pk_nil();
    PKAtom iter = list;
    while (pk_is_cons(iter)) {
        last = iter->cons.car;
        iter = iter->cons.cdr;
    }
    return last;
}

PKAtom pk_compile_evlist_except_last(PKCompiler *c, PKAtom args) {
    PKAtom iter = args;
    while (pk_is_cons(iter) && pk_is_cons(iter->cons.cdr)) {
        pk_propagate(pk_compile_value(c, iter->cons.car));
        iter = iter->cons.cdr;
    }
    return pk_true();
}

PKAtom pk_protect_begin(PKCompiler *c, bool *outer) {
    *outer = (c->protect_depth == 0);
    if (*outer) {
        pk_propagate(pk_cmp_push_byte(c, PK_OP_PROTECT_BEGIN));
    }
    c->protect_depth += 1;
    return pk_true();
}

PKAtom pk_protect_end(PKCompiler *c, bool outer) {
    c->protect_depth -= 1;
    if (outer) {
        pk_propagate(pk_cmp_push_byte(c, PK_OP_PROTECT_END));
    }
    return pk_true();
}

PKAtom pk_compile_protect(PKCompiler *c, PKAtom args) {
    bool outer = false;
    pk_propagate(pk_protect_begin(c, &outer));
    
    pk_propagate(pk_compile_evlist(c, args));
    
    pk_propagate(pk_protect_end(c, outer));
    return pk_true();
}

PKAtom pk_compile_unwind_protect(PKCompiler *c, PKAtom args) {
    pk_propagate(pk_typecheck_cons(args));
    if ((!pk_is_cons(args)) || (!pk_is_cons(args->cons.cdr))) {
        return pk_errors(pk_str("bad unwind-protect"));
    }
    
    PKAtom last = pk_cons_last(args);
    
    bool outer = false;
    pk_propagate(pk_protect_begin(c, &outer));
    
    // Every protected form runs, even when an earlier one of them throws.
    pk_propagate(pk_compile_evlist_except_last(c, args));
    
    // The cleanup form runs protected as well, so a failing one does not stop
    // anything, but its value is dropped so the protected value is the result
    // of the whole form.
    pk_propagate(pk_cmp_push_byte(c, PK_OP_BLOCK_BEGIN));
    pk_propagate(pk_compile_value(c, last));
    pk_propagate(pk_cmp_push_byte(c, PK_OP_BLOCK_POP));
    
    pk_propagate(pk_protect_end(c, outer));
    return pk_true();
}

PKAtom pk_compile_quasiquote(PKCompiler *c, PKAtom value) {
    PKAtom spliced = NULL;
    pk_propagate(pk_compile_quasiquote_value(c, value, &spliced));
    if (spliced != NULL) {
        return pk_errors(pk_str("bad quasiquote"));
    }
    return pk_true();
}

PKAtom pk_compile_jmp_rec_(PKCompiler *c, PKAtom args, pk_u8 op, PKBytes *patches) {
    pk_propagate(pk_compile_value(c, args->cons.car));
    
    if (pk_is_nil(args->cons.cdr)) {
        return pk_true();
    }
    
    pk_propagate(pk_cmp_push_byte(c, op));
    pk_propagate(pk_cmp_push_byte(c, 0));
    pk_propagate(pk_bytes_push(patches, pk_cmp_addr(c), 16));
    
    return pk_compile_jmp_rec_(c, args->cons.cdr, op, patches);
}

PKAtom pk_compile_jmp_(PKCompiler *c, PKAtom args, pk_u8 op, pk_u8 empty_op, PKBytes *patches) {
    if (!pk_is_cons(args)) {
        if (pk_is_nil(args)) {
            return pk_cmp_push_byte(c, empty_op);
        }
        return pk_error_type(PK_TYPE_SEQ, args);
    }
    
    PKAtom result = pk_true();
    
    pk_propagate(pk_cmp_push_byte(c, PK_OP_BLOCK_BEGIN));
    pk_propagate(pk_compile_jmp_rec_(c, args, op, patches));
    pk_propagate(pk_cmp_push_byte(c, PK_OP_BLOCK_END));
    
    pk_u8 end = pk_cmp_addr(c);
    for (size_t i = 0; i < patches->count; ++i) {
        result = pk_cmp_patch_byte(c, patches->e[i], end - patches->e[i]);
        if (pk_is_throwing(result)) {
            break;
        }
    }
    
    return result;
}

PKAtom pk_compile_jmp(PKCompiler *c, PKAtom args, pk_u8 op, pk_u8 empty_op) {
    PKBytes patches = PK_BYTES_EMPTY;
    PKAtom result = pk_compile_jmp_(c, args, op, empty_op, &patches);
    pk_free(patches.e);
    return result;
}

PKAtom pk_compile_progn(PKCompiler *c, PKAtom atom) {
    pk_propagate(pk_cmp_push_byte(c, PK_OP_BLOCK_BEGIN));
    pk_propagate(pk_compile_evlist(c, atom));
    pk_propagate(pk_cmp_push_byte(c, PK_OP_BLOCK_END));
    return pk_true();
}

PKAtom pk_compile_var(PKCompiler *c, PKAtom form, PK_ENV env) {
    PKAtom a = form;
    pk_propagate(pk_typecheck_cons(a));
    pk_propagate(pk_typecheck_symbol(a->cons.car));
    PKAtom b = a->cons.cdr;
    pk_propagate(pk_typecheck_cons(b));
    pk_propagate(pk_typecheck_nil(b->cons.cdr));
    pk_propagate(pk_compile_value(c, b->cons.car));
    pk_propagate(pk_cmp_load(c, a->cons.car));
    
    switch (env) {
        case PK_ENV_VAR: {
            pk_propagate(pk_cmp_push_byte(c, PK_OP_LET_VAR));
            pk_propagate(pk_cmp_push_byte(c, 1));
            break;
        }
        case PK_ENV_FUN: {
            pk_propagate(pk_cmp_push_byte(c, PK_OP_LET_FUN));
            pk_propagate(pk_cmp_push_byte(c, 1));
            break;
        }
        case PK_ENV_PROP: {
            pk_propagate(pk_cmp_push_byte(c, PK_OP_LET_PROP));
            pk_propagate(pk_cmp_push_byte(c, 1));
            break;
        }
    }
    
    return pk_true();
}

PKAtom pk_compile_special(PKCompiler *c, PKAtom symbol, PKAtom args, bool *is_special) {
    if (symbol == PK_CORE.cache.lambda) {
        pk_propagate(pk_typecheck_cons(args));
        PKAtom lfun = pk_compile_lambda(PK_CALL_FUN, args->cons.car, args->cons.cdr);
        pk_propagate(lfun);
        pk_propagate(pk_cmp_load(c, lfun));
        pk_ref_dec(lfun);
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.macro) {
        pk_propagate(pk_typecheck_cons(args));
        PKAtom lfun = pk_compile_lambda(PK_CALL_MACRO, args->cons.car, args->cons.cdr);
        pk_propagate(lfun);
        pk_propagate(pk_cmp_load(c, lfun));
        pk_ref_dec(lfun);
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.quote) {
        pk_propagate(pk_typecheck_cons(args));
        pk_propagate(pk_typecheck_nil(args->cons.cdr));
        pk_propagate(pk_cmp_load(c, args->cons.car));
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.sym_while) {
        pk_propagate(pk_typecheck_cons(args));
        
        PKAtom predicate = args->cons.car;
        PKAtom body = args->cons.cdr;

        pk_propagate(pk_cmp_push_byte(c, PK_OP_BLOCK_BEGIN));
        pk_propagate(pk_cmp_push_byte(c, PK_OP_BLOCK_CLEAR));
        
        pk_u8 start = pk_cmp_addr(c);
        
        pk_propagate(pk_compile_value(c, predicate));
        pk_propagate(pk_cmp_push_byte(c, PK_OP_JMP_UNLESS));
        pk_propagate(pk_cmp_push_byte(c, 0));
        
        pk_u8 patch = pk_cmp_addr(c);
        
        pk_propagate(pk_compile_evlist(c, body));
        pk_propagate(pk_cmp_push_byte(c, PK_OP_JMP_BACK));
        pk_propagate(pk_cmp_push_byte(c, pk_cmp_addr(c) - start + 1));
        pk_propagate(pk_cmp_patch_byte(c, patch, pk_cmp_addr(c) + 1 - patch));
        
        pk_propagate(pk_cmp_push_byte(c, PK_OP_BLOCK_END));
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.sym_if) {
        pk_propagate(pk_typecheck_cons(args));
        PKAtom value = args->cons.car;
        PKAtom a = args->cons.cdr;
        pk_propagate(pk_typecheck_cons(a));
        PKAtom clause_t = a->cons.car;
        PKAtom b = a->cons.cdr;
        pk_propagate(pk_typecheck_cons(b));
        PKAtom clause_f = b->cons.car;
        pk_propagate(pk_typecheck_nil(b->cons.cdr));

        pk_propagate(pk_cmp_push_byte(c, PK_OP_BLOCK_BEGIN));
        pk_propagate(pk_compile_value(c, value));

        pk_propagate(pk_cmp_push_byte(c, PK_OP_JMP_UNLESS));
        pk_propagate(pk_cmp_push_byte(c, 0));
        pk_u8 patch_1 = pk_cmp_addr(c);
        
        pk_propagate(pk_compile_value(c, clause_t));
        pk_propagate(pk_cmp_push_byte(c, PK_OP_JMP));
        pk_propagate(pk_cmp_push_byte(c, 0));
        pk_u8 patch_2 = pk_cmp_addr(c);
        
        pk_u8 jump_to_f = patch_2 + 1 - patch_1;
        pk_propagate(pk_compile_value(c, clause_f));
        pk_u8 jump_from_t = pk_cmp_addr(c) + 1 - patch_2;

        pk_propagate(pk_cmp_push_byte(c, PK_OP_BLOCK_END));
        
        pk_propagate(pk_cmp_patch_byte(c, patch_1, jump_to_f));
        pk_propagate(pk_cmp_patch_byte(c, patch_2, jump_from_t));
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.quasiquote) {
        pk_propagate(pk_typecheck_cons(args));
        pk_propagate(pk_typecheck_nil(args->cons.cdr));
        pk_propagate(pk_compile_quasiquote(c, args->cons.car));

        *is_special = true;
    } else if (symbol == PK_CORE.cache.unquote) {
        return pk_errors(pk_str("unquote outside quasiquote"));
    } else if (symbol == PK_CORE.cache.let) {
        pk_propagate(pk_compile_let(c, args, PK_ENV_VAR));
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.flet) {
        pk_propagate(pk_compile_let(c, args, PK_ENV_FUN));
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.let_star) {
        pk_propagate(pk_compile_let_star(c, args, PK_ENV_VAR));
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.flet_star) {
        pk_propagate(pk_compile_let_star(c, args, PK_ENV_FUN));
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.progn) {
        pk_propagate(pk_compile_progn(c, args));
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.string_substitute) {
        pk_propagate(pk_typecheck_cons(args));
        pk_propagate(pk_typecheck_string(args->cons.car));
        pk_propagate(pk_typecheck_nil(args->cons.cdr));
        pk_propagate(pk_compile_strsub(c, args->cons.car->string.str));
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.var) {
        pk_propagate(pk_compile_var(c, args, PK_ENV_VAR));
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.protect) {
        pk_propagate(pk_compile_protect(c, args));
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.unwind_protect) {
        pk_propagate(pk_compile_unwind_protect(c, args));
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.fvar) {
        pk_propagate(pk_compile_var(c, args, PK_ENV_FUN));
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.sym_and) {
        pk_propagate(pk_compile_jmp(c, args, PK_OP_JMP_UNLESS, PK_OP_LOAD_TRUE));
        
        *is_special = true;
    } else if (symbol == PK_CORE.cache.sym_or) {
        pk_propagate(pk_compile_jmp(c, args, PK_OP_JMP_WHEN, PK_OP_LOAD_NIL));
        
        *is_special = true;
    } else {
        *is_special = false;
    }
    return pk_true();
}

PKAtom pk_compile_expression(PKCompiler *c, PKAtom expr) {
    pk_propagate(pk_typecheck_cons(expr));
    PKAtom symbol = expr->cons.car;
    PKAtom body = expr->cons.cdr;
    
    pk_propagate(pk_typecheck_symbol(symbol));

    {
        bool is_special = false;
        pk_propagate(pk_compile_special(c, symbol, body, &is_special));

        if (is_special) {
            return pk_true();
        }
    }
    
    pk_propagate(pk_cmp_load(c, symbol));

    size_t acc = 0;
    PKAtom iter = body;
    while (pk_is_cons(iter)) {
        pk_propagate(pk_compile_value(c, iter->cons.car));
        acc += 1;
        iter = iter->cons.cdr;
    }

    if (acc >= UINT8_MAX) {
        return pk_errors(pk_str("too many expressions"));
    }

    pk_propagate(pk_cmp_push_byte(c, PK_OP_CALL));
    pk_propagate(pk_cmp_push_byte(c, (pk_u8)acc));
    
    return pk_true();
}

PKAtom pk_compile_value(PKCompiler *c, PKAtom value) {
    switch (value->header.type) {
        case PK_ATOMTY_CONS: {
            return pk_compile_expression(c, value);
        }
        case PK_ATOMTY_SYMBOL: {
            return pk_cmp_lookup(c, value, PK_ENV_VAR);
        }
        default: {
            return pk_cmp_load(c, value);
        }
    }
}

PKAtom pk_compile_compile(PKCompiler *c, pk_u8 args, pk_u8 arity, pk_u8 option_start) {
    PKAtom result = pk_atom_alloc();
    pk_propagate(result);
    
    for (size_t i = c->atoms.count; i < c->atoms.capacity; ++i) {
        c->atoms.e[i] = pk_nil();
    }
    
    for (size_t i = c->bc.count; i < c->bc.capacity; ++i) {
        c->bc.e[i] = PK_OP_ILLEGAL;
    }

    if (c->type == PK_CALL_FUN) {
        result->header.type = PK_ATOMTY_LFUN;
    } else {
        result->header.type = PK_ATOMTY_LMACRO;
    }
    result->lfun.atoms.e = c->atoms.e;
    result->lfun.atoms.length = c->atoms.capacity;
    result->lfun.bc.e = c->bc.e;
    result->lfun.bc.length = c->bc.capacity;
    result->lfun.args = args;
    result->lfun.arity = arity;
    result->lfun.option_start = option_start;

    // pk_propagate(pk_hexdump(result));

    return result;
}

PKAtom pk_compile_lambda_(PKCompiler *c, PKAtom args, PKAtom body) {
    pk_u8 nargs = 0;
    pk_u8 arity = PK_INT_ARITY_NORMAL;
    pk_u8 option_start = 0;
    bool variadic = false;
    
    {
        PKAtom iter = args;
        while (pk_is_cons(iter)) {
            PKAtom atom = iter->cons.car;
            if (pk_is_keyword(atom)) {
                if (variadic) {
                    return pk_errors(pk_str("bad keyword arg"));
                }
                if (pk_keyword_qeq(atom, pk_str("rest"))) {
                    arity = PK_INT_ARITY_VARIADIC;
                    variadic = true;
                } else if (pk_keyword_qeq(atom, pk_str("opt"))) {
                    if (arity != PK_INT_ARITY_OPTIONAL) {
                        option_start = nargs;
                    }
                    arity = PK_INT_ARITY_OPTIONAL;
                } else if (pk_keyword_qeq(atom, pk_str("optional"))) {
                    if (arity != PK_INT_ARITY_OPTIONAL) {
                        option_start = nargs;
                    }
                    arity = PK_INT_ARITY_OPTIONAL;
                }
            } else {
                pk_propagate(pk_typecheck_symbol(atom));
                pk_propagate(pk_cmp_load(c, atom));

                if (variadic) {
                    pk_propagate(pk_typecheck_nil(iter->cons.cdr));
                }

                if (nargs == (UINT8_MAX - 1)) {
                    return pk_errors(pk_str("too many args"));
                }
                nargs += 1;
            }

            iter = iter->cons.cdr;
        }
    }

    if (arity != PK_INT_ARITY_OPTIONAL) {
        option_start = nargs;
    }

    if (nargs > 0) {
        pk_propagate(pk_cmp_push_byte(c, PK_OP_LET_VAR));
        pk_propagate(pk_cmp_push_byte(c, nargs));
    }
    
    pk_propagate(pk_compile_evlist(c, body));
    pk_propagate(pk_cmp_push_byte(c, PK_OP_RET));

    return pk_compile_compile(c, nargs, arity, option_start);
}
    
PKAtom pk_compile_lambda(PK_CALL type, PKAtom args, PKAtom body) {
    PKCompiler c = pk_cmp_new(type);
    PKAtom result = pk_compile_lambda_(&c, args, body);
    if (pk_is_throwing(result)) {
        pk_cmp_deinit(&c);
    }
    return result;
}

PKAtom pk_compile_atom(PKCompiler *c, PKAtom value) {
    pk_propagate(pk_compile_value(c, value));
    pk_propagate(pk_cmp_push_byte(c, PK_OP_RET));
    
    return pk_compile_compile(c, 0, PK_INT_ARITY_NORMAL, 0);
}

PKAtom pk_compile_exprlist(PKCompiler *c, PKAtom exprlist) {
    pk_propagate(pk_compile_progn(c, exprlist));
    pk_propagate(pk_cmp_push_byte(c, PK_OP_RET));
    return pk_compile_compile(c, 0, PK_INT_ARITY_NORMAL, 0);
}
    
PKAtom pk_compile(PK_CALL type, PKAtom value, PK_MODE mode) {
    PKCompiler c = pk_cmp_new(type);
    PKAtom result = pk_nil();
    
    if (mode == PK_MODE_EXPRESSION) {
        result = pk_compile_atom(&c, value);
    } else {
        result = pk_compile_exprlist(&c, value);
    }
    
    if (pk_is_throwing(result)) {
        pk_cmp_deinit(&c);
    }
    return result;
}
