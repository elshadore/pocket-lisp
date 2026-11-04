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
        .transferred = false,
    };
}

#define pk_cmp_addr(c_) ((c_)->addr)

void pk_cmp_patch_byte(Pocket lisp, PKCompiler *c, pk_u8 addr, pk_u8 byte) {
    if (addr >= c->bc.count) {
        pk_errors(lisp, pk_str("bad byte address"));
    }
    c->bc.e[addr] = byte;
}

void pk_cmp_push_byte(Pocket lisp, PKCompiler *c, pk_u8 byte) {
    if (c->bc.count >= UINT8_MAX) {
        pk_errors(lisp, pk_str("bytecode too big"));
    }

    c->addr = (pk_u8)c->bc.count;

    pk_bytes_push(lisp, &c->bc, byte, 8);
}

void pk_cmp_push_any(Pocket lisp, PKCompiler *c, size_t value) {
    if (value > UINT8_MAX) {
        pk_errors(lisp, pk_str("address too big"));
    }

    pk_cmp_push_byte(lisp, c, (pk_u8)value);
}

void pk_cmp_push_atom(Pocket lisp, PKCompiler *c, PKAtom *atom) {
    if (c->atoms.count >= UINT8_MAX) {
        pk_errors(lisp, pk_str("too many atoms"));
    }

    for (size_t i = 0; i < c->atoms.count; ++i) {
        PKAtom *cmp = c->atoms.e[i];
        if (pk_atom_eq(lisp, atom, cmp)) {
            c->addr = (pk_u8)i;
            return;
        }
    }

    c->addr = (pk_u8)c->atoms.count;
    pk_array_push(lisp, &c->atoms, atom, 8);
}

void pk_cmp_load_nil(Pocket lisp, PKCompiler *c) {
    pk_cmp_push_byte(lisp, c, PK_OP_LOAD_NIL);
}

void pk_cmp_load_true(Pocket lisp, PKCompiler *c) {
    pk_cmp_push_byte(lisp, c, PK_OP_LOAD_TRUE);
}

void pk_cmp_load(Pocket lisp, PKCompiler *c, PKAtom *atom) {
    if (pk_atom_is_nil(atom)) {
        pk_cmp_load_nil(lisp, c);
        return;
    }
    if (atom->header.type == PK_ATOMTY_TRUE) {
        pk_cmp_load_true(lisp, c);
        return;
    }

    pk_cmp_push_atom(lisp, c, atom);
    pk_u8 addr = pk_cmp_addr(c);

    pk_cmp_push_byte(lisp, c, PK_OP_LOAD);
    pk_cmp_push_byte(lisp, c, addr);
}

void pk_cmp_lookup(Pocket lisp, PKCompiler *c, PKAtomSymbol *symbol, PK_ENV env) {
    pk_cmp_push_atom(lisp, c, (PKAtom *)symbol);
    pk_u8 addr = pk_cmp_addr(c);

    switch (env) {
        case PK_ENV_VAR: {
            pk_cmp_push_byte(lisp, c, PK_OP_GET_VAR);
            break;
        }
        case PK_ENV_FUN: {
            pk_cmp_push_byte(lisp, c, PK_OP_GET_FUN);
            break;
        }
    }

    pk_cmp_push_byte(lisp, c, addr);
}

void pk_cmp_deinit(Pocket lisp, PKCompiler *c) {
    if (!c->transferred) {
        pk_free(lisp, c->atoms.e);
        pk_free(lisp, c->bc.e);
    }
    PK_CALL type = c->type;
    *c = pk_cmp_new(type);
}

void pk_compile_evlist(Pocket lisp, PKCompiler *c, PKAtom *args);

void pk_compile_value(Pocket lisp, PKCompiler *c, PKAtom *value);
PKAtomLFun *pk_compile_lambda_(Pocket lisp, PKCompiler *c, PKAtom *args, PKAtom *body);
PKAtomLFun *pk_compile_lambda(Pocket lisp, PK_CALL type, PKAtom *args, PKAtom *body);
void pk_compile_strsub(Pocket lisp, PKCompiler *c, PKStr string);

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

void pk_compile_pattern_path(Pocket lisp, PKCompiler *c, pk_u8 *path, size_t length, bool optional) {
    for (size_t i = 0; i < length; ++i) {
        if (path[i] == PK_PATTERN_PATH_CAR) {
            pk_cmp_push_byte(lisp, c, pk_pattern_car_op(optional));
        } else {
            pk_cmp_push_byte(lisp, c, pk_pattern_cdr_op(optional));
        }
    }
}

void pk_compile_pattern_cdrs(Pocket lisp, PKCompiler *c, size_t index, bool optional) {
    pk_u8 op = pk_pattern_cdr_op(optional);
    for (size_t i = 0; i < index; ++i) {
        pk_cmp_push_byte(lisp, c, op);
    }
}

void pk_compile_pattern_descend(Pocket lisp, pk_u8 *path, size_t length, size_t index) {
    if ((length + index + 1) >= PK_PATTERN_PATH_MAX) {
        pk_errors(lisp, pk_str("destructure too deep"));
    }

    for (size_t i = 0; i < index; ++i) {
        path[length + i] = PK_PATTERN_PATH_CDR;
    }
    path[length + index] = PK_PATTERN_PATH_CAR;
}

void pk_compile_pattern_values_(Pocket lisp, PKCompiler *c, PKAtom *pattern, pk_u8 *path, size_t length, size_t base, size_t bias, size_t *pushed, bool optional) {
    PKAtom *iter = pattern;
    size_t index = 0;
    while (pk_atom_is_cons(iter)) {
        PKAtom *atom = iter->cons.car;
        PKAtom *tail = iter->cons.cdr;

        if (pk_atom_is_keyword(atom)) {
            if ((pk_atom_keyword_cmp((PKAtomKeyword *)atom, pk_str("opt"))) ||
                (pk_atom_keyword_cmp((PKAtomKeyword *)atom, pk_str("optional")))) {
                optional = true;
                if ((!pk_atom_is_cons(tail)) || (!pk_atom_is_symbol(tail->cons.car))) {
                    pk_errors(lisp, pk_str("bad destructure"));
                }
            } else if (pk_atom_keyword_cmp((PKAtomKeyword *)atom, pk_str("rest"))) {
                if ((!pk_atom_is_cons(tail)) || (!pk_atom_is_symbol(tail->cons.car)) || (!pk_atom_is_nil(tail->cons.cdr))) {
                    pk_errors(lisp, pk_str("bad destructure"));
                }

                pk_cmp_push_byte(lisp, c, PK_OP_DIG);
                pk_cmp_push_any(lisp, c, (*pushed - base) + bias);
                pk_compile_pattern_path(lisp, c, path, length, optional);
                pk_compile_pattern_cdrs(lisp, c, index, true);
                *pushed += 1;
                return;
            } else {
                pk_errors(lisp, pk_str("bad destructure"));
            }
        } else if (pk_atom_is_symbol(atom)) {
            pk_cmp_push_byte(lisp, c, PK_OP_DIG);
            pk_cmp_push_any(lisp, c, (*pushed - base) + bias);
            pk_compile_pattern_path(lisp, c, path, length, optional);
            pk_compile_pattern_cdrs(lisp, c, index, optional);
            pk_cmp_push_byte(lisp, c, pk_pattern_car_op(optional));
            *pushed += 1;
        } else if (pk_atom_is_cons(atom)) {
            if (optional) {
                pk_errors(lisp, pk_str("bad destructure"));
            }
            pk_compile_pattern_descend(lisp, path, length, index);
            pk_compile_pattern_values_(lisp, c, atom, path, length + index + 1, base, bias, pushed, optional);
        } else {
            pk_errors(lisp, pk_str("bad destructure"));
        }

        iter = tail;
        if (!pk_atom_is_keyword(atom)) {
            index += 1;
        }
    }

    if (!pk_atom_is_nil(iter)) {
        pk_errors(lisp, pk_str("bad destructure"));
    }
}

void pk_compile_pattern_values(Pocket lisp, PKCompiler *c, PKAtom *pattern, size_t bias, bool optional, size_t *pushed) {
    pk_u8 path[PK_PATTERN_PATH_MAX] = {0};
    pk_compile_pattern_values_(lisp, c, pattern, path, 0, 0, bias, pushed, optional);
}

void pk_compile_pattern_symbols_(Pocket lisp, PKCompiler *c, PKAtom *pattern, bool optional, size_t *leaves) {
    PKAtom *iter = pattern;
    while (pk_atom_is_cons(iter)) {
        PKAtom *atom = iter->cons.car;
        PKAtom *tail = iter->cons.cdr;

        if (pk_atom_is_keyword(atom)) {
            if ((pk_atom_keyword_cmp((PKAtomKeyword *)atom, pk_str("opt"))) ||
                (pk_atom_keyword_cmp((PKAtomKeyword *)atom, pk_str("optional")))) {
                optional = true;
                if ((!pk_atom_is_cons(tail)) || (!pk_atom_is_symbol(tail->cons.car))) {
                    pk_errors(lisp, pk_str("bad destructure"));
                }
            } else if (pk_atom_keyword_cmp((PKAtomKeyword *)atom, pk_str("rest"))) {
                if ((!pk_atom_is_cons(tail)) || (!pk_atom_is_symbol(tail->cons.car)) || (!pk_atom_is_nil(tail->cons.cdr))) {
                    pk_errors(lisp, pk_str("bad destructure"));
                }
                pk_cmp_load(lisp, c, tail->cons.car);
                *leaves += 1;
                return;
            } else {
                pk_errors(lisp, pk_str("bad destructure"));
            }
        } else if (pk_atom_is_symbol(atom)) {
            pk_cmp_load(lisp, c, atom);
            *leaves += 1;
        } else if (pk_atom_is_cons(atom)) {
            if (optional) {
                pk_errors(lisp, pk_str("bad destructure"));
            }
            pk_compile_pattern_symbols_(lisp, c, atom, optional, leaves);
        } else {
            pk_errors(lisp, pk_str("bad destructure"));
        }

        iter = tail;
    }

    if (!pk_atom_is_nil(iter)) {
        pk_errors(lisp, pk_str("bad destructure"));
    }
}

void pk_compile_pattern_symbols(Pocket lisp, PKCompiler *c, PKAtom *pattern, bool optional, size_t *leaves) {
    pk_compile_pattern_symbols_(lisp, c, pattern, optional, leaves);
}

bool pk_let_binding_is_pattern(PKAtom *binding) {
    return pk_atom_is_cons(binding) && pk_atom_is_cons(binding->cons.car);
}

bool pk_compile_let_has_patterns(PKAtom *bindings) {
    PKAtom *iter = bindings;
    while (pk_atom_is_cons(iter)) {
        if (pk_let_binding_is_pattern(iter->cons.car)) {
            return true;
        }
        iter = iter->cons.cdr;
    }
    return false;
}

void pk_compile_let_lets(Pocket lisp, PKCompiler *c, PK_ENV env) {
    switch (env) {
        case PK_ENV_VAR: {
            pk_cmp_push_byte(lisp, c, PK_OP_LET_VAR);
            break;
        }
        case PK_ENV_FUN: {
            pk_cmp_push_byte(lisp, c, PK_OP_LET_FUN);
            break;
        }
    }
}

void pk_compile_bind(Pocket lisp, PKCompiler *c, PKAtom *bind, PK_ENV env) {
    switch (bind->header.type) {
        case PK_ATOMTY_CONS: {
            size_t pushed = 0;
            pk_compile_pattern_values(lisp, c, bind, 0, false, &pushed);

            size_t lets = 0;
            pk_compile_pattern_symbols(lisp, c, bind, false, &lets);

            pk_compile_let_lets(lisp, c, env);
            pk_cmp_push_any(lisp, c, lets);
            break;
        }
        case PK_ATOMTY_NIL: break;
        default: {
            pk_error_type2(lisp, bind, lisp->cache.type_cons, lisp->cache.type_nil);
            break;
        }
    }
}

void pk_compile_let_binding_value(Pocket lisp, PKCompiler *c, PKAtom *binding) {
    if (pk_atom_is_symbol(binding)) {
        pk_cmp_load_nil(lisp, c);
        return;
    }

    PKAtomCons *a = pk_atom_assert_cons(lisp, binding);
    PKAtomCons *b = pk_atom_assert_cons(lisp, a->cdr);
    if (!pk_atom_is_nil(b->cdr)) {
        pk_errors(lisp, pk_str("bad body"));
    }
    pk_compile_value(lisp, c, b->car);
}

void pk_compile_let(Pocket lisp, PKCompiler *c, PKAtom *args, PK_ENV env) {
    PKAtomCons *a = pk_atom_assert_cons(lisp, args);

    PKAtom *bindings = a->car;
    PKAtom *body = a->cdr;

    pk_cmp_push_byte(lisp, c, PK_OP_BLOCK_BEGIN);

    bool patterns = pk_compile_let_has_patterns(bindings);

    size_t values = 0;
    {
        PKAtom *iter = bindings;
        while (pk_atom_is_cons(iter)) {
            pk_compile_let_binding_value(lisp, c, iter->cons.car);
            values += 1;
            iter = iter->cons.cdr;
        }
    }

    if (patterns) {
        size_t pushed = 0;
        size_t index = 0;
        PKAtom *iter = bindings;
        while (pk_atom_is_cons(iter)) {
            PKAtom *binding = iter->cons.car;
            if (pk_let_binding_is_pattern(binding)) {
                pk_compile_pattern_values(lisp, c, binding->cons.car, (values - 1) - index, false, &pushed);
            } else {
                pk_cmp_push_byte(lisp, c, PK_OP_DIG);
                pk_cmp_push_any(lisp, c, pushed + ((values - 1) - index));
                pushed += 1;
            }
            index += 1;
            iter = iter->cons.cdr;
        }
    }

    size_t lets = 0;
    {
        PKAtom *iter = bindings;
        while (pk_atom_is_cons(iter)) {
            PKAtom *binding = iter->cons.car;
            if (pk_let_binding_is_pattern(binding)) {
                pk_compile_pattern_symbols(lisp, c, binding->cons.car, false, &lets);
            } else {
                PKAtom *symbol = lisp->cache.nil;
                if (pk_atom_is_symbol(binding)) {
                    symbol = binding;
                } else {
                    symbol = (PKAtom *)pk_atom_assert_symbol(lisp, pk_atom_assert_cons(lisp, binding)->car);
                }
                pk_cmp_load(lisp, c, symbol);
                lets += 1;
            }

            iter = iter->cons.cdr;
        }
    }

    pk_compile_let_lets(lisp, c, env);
    pk_cmp_push_any(lisp, c, lets);
    pk_compile_evlist(lisp, c, body);
    pk_cmp_push_byte(lisp, c, PK_OP_BLOCK_END);
}

void pk_compile_let_star(Pocket lisp, PKCompiler *c, PKAtom *args, PK_ENV env) {
    if (!pk_atom_is_cons(args)) {
        pk_errors(lisp, pk_str("bad let args"));
    }

    PKAtomCons *a = pk_atom_assert_cons(lisp, args);

    PKAtom *bindings = a->car;
    PKAtom *body = a->cdr;

    pk_cmp_push_byte(lisp, c, PK_OP_BLOCK_BEGIN);

    PKAtom *iter = bindings;
    while (pk_atom_is_cons(iter)) {
        PKAtom *binding = iter->cons.car;

        if (pk_let_binding_is_pattern(binding)) {
            PKAtom *pa = binding->cons.car;
            PKAtomCons *b = pk_atom_assert_cons(lisp, binding->cons.cdr);
            if (!pk_atom_is_nil(b->cdr)) {
                pk_errors(lisp, pk_str("bad let binding"));
            }

            pk_compile_value(lisp, c, b->car);

            size_t pushed = 0;
            pk_compile_pattern_values(lisp, c, pa, 0, false, &pushed);

            size_t lets = 0;
            pk_compile_pattern_symbols(lisp, c, pa, false, &lets);

            pk_compile_let_lets(lisp, c, env);
            pk_cmp_push_any(lisp, c, lets);

            iter = iter->cons.cdr;
            continue;
        }

        PKAtom *symbol = lisp->cache.nil;
        if (pk_atom_is_symbol(binding)) {
            pk_cmp_load_nil(lisp, c);
            symbol = binding;
        } else {
            PKAtomCons *ba = pk_atom_assert_cons(lisp, binding);
            PKAtomCons *b = pk_atom_assert_cons(lisp, ba->cdr);
            if (!pk_atom_is_nil(b->cdr)) {
                pk_errors(lisp, pk_str("bad let binding"));
            }
            symbol = (PKAtom *)pk_atom_assert_symbol(lisp, ba->car);
            pk_compile_value(lisp, c, b->car);
        }

        pk_cmp_load(lisp, c, symbol);
        pk_compile_let_lets(lisp, c, env);
        pk_cmp_push_byte(lisp, c, 1);

        iter = iter->cons.cdr;
    }

    pk_compile_evlist(lisp, c, body);
    pk_cmp_push_byte(lisp, c, PK_OP_BLOCK_END);
}

void pk_compile_quasiquote(Pocket lisp, PKCompiler *c, PKAtom *value);
void pk_compile_quasiquote_list(Pocket lisp, PKCompiler *c, PKAtom *list);
void pk_compile_quasiquote_value(Pocket lisp, PKCompiler *c, PKAtom *value, PKAtom **spliced);

void pk_compile_quasiquote_value(Pocket lisp, PKCompiler *c, PKAtom *value, PKAtom **spliced) {
    if (!pk_atom_is_cons(value)) {
        pk_cmp_load(lisp, c, value);
        return;
    }

    PKAtom *a = value->cons.car;

    if (pk_atom_is_symbol(a)) {
        PKAtomSymbol *asymbol = (PKAtomSymbol *)a;

        if (asymbol == lisp->cache.quote) {
            PKAtomCons *quoted = pk_atom_assert_cons(lisp, value->cons.cdr);
            (void)pk_atom_assert_nil(lisp, quoted->cdr);
            pk_cmp_load(lisp, c, (PKAtom *)lisp->cache.quote);
            pk_compile_quasiquote_value(lisp, c, quoted->car, spliced);
            pk_cmp_push_byte(lisp, c, PK_OP_MAKE_LIST);
            pk_cmp_push_byte(lisp, c, 2);
            return;
        }

        if (asymbol == lisp->cache.unquote) {
            PKAtom *b = value->cons.cdr;
            if (!pk_atom_is_cons(b)) {
                pk_errors(lisp, pk_str("bad unquote"));
            }
            if (!pk_atom_is_nil(b->cons.cdr)) {
                pk_errors(lisp, pk_str("bad unquote"));
            }
            pk_compile_value(lisp, c, b->cons.car);
            return;
        } else if (asymbol == lisp->cache.unquote_splice) {
            PKAtom *b = value->cons.cdr;
            if (!pk_atom_is_cons(b)) {
                pk_errors(lisp, pk_str("bad unquote splice"));
            }
            if (!pk_atom_is_nil(b->cons.cdr)) {
                pk_errors(lisp, pk_str("bad unquote splice"));
            }
            *spliced = b->cons.car;
            return;
        }
    }
    pk_compile_quasiquote_list(lisp, c, value);
}

void pk_compile_qqlist(Pocket lisp, PKCompiler *c, PKQQ *qq) {
    if (qq->packed_cons) {
        pk_cmp_push_byte(lisp, c, PK_OP_MAKE_LIST_PACKED);
    } else {
        pk_cmp_push_byte(lisp, c, PK_OP_MAKE_LIST);
    }
    pk_cmp_push_any(lisp, c, qq->count);

    qq->packed_cons = false;
    qq->count = 0;
    qq->spliced_count += 1;
}

void pk_compile_quasiquote_list(Pocket lisp, PKCompiler *c, PKAtom *list) {
    PKQQ qq = PK_QQ_EMPTY;

    PKAtom *iter = list;

    while (!pk_atom_is_nil(iter)) {

        PKAtom *spliced = NULL;

        if (pk_atom_is_cons(iter)) {
            pk_compile_quasiquote_value(lisp, c, iter->cons.car, &spliced);
            iter = iter->cons.cdr;
        } else {
            pk_compile_quasiquote_value(lisp, c, iter, &spliced);
            qq.packed_cons = true;
            iter = lisp->cache.nil;
        }

        if (spliced != NULL) {
            qq.spliced = true;
            qq.spliced_last = true;
            pk_compile_qqlist(lisp, c, &qq);
            pk_compile_value(lisp, c, spliced);
        } else {
            qq.spliced_last = false;
            qq.count += 1;
        }
    }

    if (!qq.spliced_last) {
        pk_compile_qqlist(lisp, c, &qq);
    }

    if (qq.spliced) {
        pk_cmp_push_byte(lisp, c, PK_OP_MERGE_LISTS);
        pk_cmp_push_any(lisp, c, qq.spliced_count + 1);
    }
}

PKAtom *pk_cons_last(Pocket lisp, PKAtom *list) {
    PKAtom *last = lisp->cache.nil;
    PKAtom *iter = list;
    while (pk_atom_is_cons(iter)) {
        last = iter->cons.car;
        iter = iter->cons.cdr;
    }
    return last;
}

void pk_compile_evlist_except_last(Pocket lisp, PKCompiler *c, PKAtom *args) {
    PKAtom *iter = args;
    while (pk_atom_is_cons(iter) && pk_atom_is_cons(iter->cons.cdr)) {
        pk_compile_value(lisp, c, iter->cons.car);
        iter = iter->cons.cdr;
    }
}

void pk_compile_set(Pocket lisp, PKCompiler *c, PKAtom *args, PK_ENV env) {
    PKAtomCons *a = pk_atom_assert_cons(lisp, args);
    PKAtomCons *b = pk_atom_assert_cons(lisp, a->cdr);
    (void)pk_atom_assert_nil(lisp, b->cdr);
    pk_atom_assert_symbol(lisp, a->car);

    pk_cmp_load(lisp, c, a->car);
    pk_compile_value(lisp, c, b->car);

    switch (env) {
        case PK_ENV_VAR: {
            pk_cmp_push_byte(lisp, c, PK_OP_SET_VAR);
            break;
        }
        case PK_ENV_FUN: {
            pk_cmp_push_byte(lisp, c, PK_OP_SET_FUN);
            break;
        }
    }
}

void pk_compile_protect_evlist(Pocket lisp, PKCompiler *c, PKAtom *args) {
    PKAtom *iter = args;

    while (pk_atom_is_cons(iter)) {
        PKAtom *lfun = pk_atom_compile(lisp, PK_CALL_FUN, iter->cons.car, PK_MODE_EXPRESSION);
        pk_cmp_load(lisp, c, lfun);
        pk_cmp_push_byte(lisp, c, PK_OP_CALL);
        pk_cmp_push_byte(lisp, c, 0);
        iter = iter->cons.cdr;
    }
}

void pk_compile_protect(Pocket lisp, PKCompiler *c, PKAtom *args) {
    pk_cmp_push_byte(lisp, c, PK_OP_PROTECT_BEGIN);
    pk_compile_protect_evlist(lisp, c, args);
    pk_cmp_push_byte(lisp, c, PK_OP_PROTECT_END);
}

void pk_compile_quasiquote(Pocket lisp, PKCompiler *c, PKAtom *value) {
    PKAtom *spliced = NULL;
    pk_compile_quasiquote_value(lisp, c, value, &spliced);
    if (spliced != NULL) {
        pk_errors(lisp, pk_str("bad quasiquote"));
    }
}

void pk_compile_jmp_rec_(Pocket lisp, PKCompiler *c, PKAtom *args, pk_u8 op, PKBytes *patches) {
    pk_compile_value(lisp, c, args->cons.car);

    if (pk_atom_is_nil(args->cons.cdr)) {
        return;
    }

    pk_cmp_push_byte(lisp, c, op);
    pk_cmp_push_byte(lisp, c, 0);
    pk_bytes_push(lisp, patches, pk_cmp_addr(c), 16);

    pk_compile_jmp_rec_(lisp, c, args->cons.cdr, op, patches);
}

void pk_compile_jmp_(Pocket lisp, PKCompiler *c, PKAtom *args, pk_u8 op, pk_u8 empty_op, PKBytes *patches) {
    if (!pk_atom_is_cons(args)) {
        if (pk_atom_is_nil(args)) {
            pk_cmp_push_byte(lisp, c, empty_op);
            return;
        } else {
            pk_error_type2(lisp, args, lisp->cache.type_cons, lisp->cache.type_nil);
            return;
        }
    }

    pk_cmp_push_byte(lisp, c, PK_OP_BLOCK_BEGIN);
    pk_compile_jmp_rec_(lisp, c, args, op, patches);
    pk_cmp_push_byte(lisp, c, PK_OP_BLOCK_END);

    pk_u8 end = pk_cmp_addr(c);
    for (size_t i = 0; i < patches->count; ++i) {
        pk_cmp_patch_byte(lisp, c, patches->e[i], end - patches->e[i]);
    }
}

typedef struct PKCompileJmpData_ {
    PKBytes patches;
    PKCompiler *c;
    PKAtom *args;
    pk_u8 op;
    pk_u8 empty_op;
} PKCompileJmpData;

void pk_compile_jmp_body(void *user_closure, Pocket lisp) {
    PKCompileJmpData *data = (PKCompileJmpData *)user_closure;
    pk_compile_jmp_(lisp, data->c, data->args, data->op, data->empty_op, &data->patches);
}

void pk_compile_jmp(Pocket lisp, PKCompiler *c, PKAtom *args, pk_u8 op, pk_u8 empty_op) {
    PKCompileJmpData data = (PKCompileJmpData) {
        .patches = PK_BYTES_EMPTY,
        .c = c,
        .args = args,
        .op = op,
        .empty_op = empty_op,
    };

    PK_RES result = pk_protect(lisp, &data, pk_compile_jmp_body, 0);
    pk_free(lisp, data.patches.e);

    if (result == PK_YIELDED) {
        pk_propagate(lisp);
    }
}

void pk_compile_progn(Pocket lisp, PKCompiler *c, PKAtom *atom) {
    pk_cmp_push_byte(lisp, c, PK_OP_BLOCK_BEGIN);
    pk_compile_evlist(lisp, c, atom);
    pk_cmp_push_byte(lisp, c, PK_OP_BLOCK_END);
}

void pk_compile_var(Pocket lisp, PKCompiler *c, PKAtom *form, PK_ENV env) {
    PKAtomCons *a = pk_atom_assert_cons(lisp, form);
    PKAtomCons *b = pk_atom_assert_cons(lisp, a->cdr);
    (void)pk_atom_assert_nil(lisp, b->cdr);

    pk_compile_value(lisp, c, b->car);

    if (pk_atom_is_cons(a->car)) {
        pk_compile_bind(lisp, c, a->car, env);
    } else {
        pk_atom_assert_symbol(lisp, a->car);

        pk_cmp_push_byte(lisp, c, PK_OP_DIG);
        pk_cmp_push_any(lisp, c, 0);

        pk_cmp_load(lisp, c, a->car);

        switch (env) {
            case PK_ENV_VAR: {
                pk_cmp_push_byte(lisp, c, PK_OP_LET_VAR);
                pk_cmp_push_byte(lisp, c, 1);
                break;
            }
            case PK_ENV_FUN: {
                pk_cmp_push_byte(lisp, c, PK_OP_LET_FUN);
                pk_cmp_push_byte(lisp, c, 1);
                break;
            }
        }
    }
}

void pk_compile_defclass(Pocket lisp, PKCompiler *c, PKAtom *form) {
    // Our ideal form is:
    
    // (defclass example ()
    //   foo
    //   bar
    //   baz)
    
    // We will keep the () after defclass *symbol* for eventually adding inheritance.
    
    PKAtomCons *a = pk_atom_assert_cons(lisp, form);
    PKAtom *name = (PKAtom *)pk_atom_assert_symbol(lisp, a->car);
    PKAtomCons *b = pk_atom_assert_cons(lisp, a->cdr);
    (void)pk_atom_assert_nil(lisp, b->car);
    PKAtom *slots = b->cdr;

    pk_cmp_load(lisp, c, name);
    pk_cmp_load(lisp, c, slots);
    pk_cmp_push_byte(lisp, c, PK_OP_DEFCLASS);
}

void pk_compile_special(Pocket lisp, PKCompiler *c, PKAtomSymbol *symbol, PKAtom *args, bool *is_special) {

    if (symbol == lisp->cache.lambda) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);
        PKAtomLFun *lfun = pk_compile_lambda(lisp, PK_CALL_FUN, a->car, a->cdr);
        pk_cmp_load(lisp, c, (PKAtom *)lfun);

        *is_special = true;
    } else if (symbol == lisp->cache.macro) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);
        PKAtomLFun *lfun = pk_compile_lambda(lisp, PK_CALL_MACRO, a->car, a->cdr);
        pk_cmp_load(lisp, c, (PKAtom *)lfun);

        *is_special = true;
    } else if ((symbol == lisp->cache.defun) || (symbol == lisp->cache.defmacro)) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);
        (void)pk_atom_assert_symbol(lisp, a->car);
        PKAtomCons *b = pk_atom_assert_cons(lisp, a->cdr);

        PK_CALL call = PK_CALL_FUN;
        if (symbol == lisp->cache.defmacro) {
            call = PK_CALL_MACRO;
        }

        PKAtomLFun *lfun = pk_compile_lambda(lisp, call, b->car, b->cdr);

        pk_cmp_load(lisp, c, a->car);
        pk_cmp_load(lisp, c, (PKAtom *)lfun);
        pk_cmp_push_byte(lisp, c, PK_OP_SET_FUN);
        pk_cmp_load(lisp, c, a->car);

        *is_special = true;
    } else if (symbol == lisp->cache.defvar) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);
        (void)pk_atom_assert_symbol(lisp, a->car);

        pk_cmp_load(lisp, c, a->car);

        if (pk_atom_is_cons(a->cdr)) {
            PKAtomCons *b = pk_atom_assert_cons(lisp, a->cdr);
            (void)pk_atom_assert_nil(lisp, b->cdr);
            pk_compile_value(lisp, c, b->car);
        } else {
            (void)pk_atom_assert_nil(lisp, a->cdr);
            pk_cmp_load_nil(lisp, c);
        }

        pk_cmp_push_byte(lisp, c, PK_OP_SET_VAR);
        pk_cmp_load(lisp, c, a->car);

        *is_special = true;
    } else if (symbol == lisp->cache.quote) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);
        (void)pk_atom_assert_nil(lisp, a->cdr);
        pk_cmp_load(lisp, c, a->car);

        *is_special = true;
    } else if (symbol == lisp->cache.sym_while) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);

        PKAtom *predicate = a->car;
        PKAtom *body = a->cdr;

        pk_cmp_push_byte(lisp, c, PK_OP_BLOCK_BEGIN);
        pk_cmp_push_byte(lisp, c, PK_OP_BLOCK_CLEAR);

        pk_u8 start = pk_cmp_addr(c);

        pk_compile_value(lisp, c, predicate);
        pk_cmp_push_byte(lisp, c, PK_OP_JMP_UNLESS);
        pk_cmp_push_byte(lisp, c, 0);

        pk_u8 patch = pk_cmp_addr(c);

        pk_compile_evlist(lisp, c, body);
        pk_cmp_push_byte(lisp, c, PK_OP_JMP_BACK);
        pk_cmp_push_byte(lisp, c, pk_cmp_addr(c) - start + 1);
        pk_cmp_patch_byte(lisp, c, patch, pk_cmp_addr(c) + 1 - patch);

        pk_cmp_push_byte(lisp, c, PK_OP_BLOCK_END);

        *is_special = true;
    } else if (symbol == lisp->cache.sym_if) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);
        PKAtom *value = a->car;
        PKAtomCons *b = pk_atom_assert_cons(lisp, a->cdr);
        PKAtom *clause_t = b->car;
        PKAtomCons *d = pk_atom_assert_cons(lisp, b->cdr);
        PKAtom *clause_f = d->car;
        (void)pk_atom_assert_nil(lisp, d->cdr);

        pk_cmp_push_byte(lisp, c, PK_OP_BLOCK_BEGIN);
        pk_compile_value(lisp, c, value);

        pk_cmp_push_byte(lisp, c, PK_OP_JMP_UNLESS);
        pk_cmp_push_byte(lisp, c, 0);
        pk_u8 patch_1 = pk_cmp_addr(c);

        pk_compile_value(lisp, c, clause_t);
        pk_cmp_push_byte(lisp, c, PK_OP_JMP);
        pk_cmp_push_byte(lisp, c, 0);
        pk_u8 patch_2 = pk_cmp_addr(c);

        pk_u8 jump_to_f = patch_2 + 1 - patch_1;
        pk_compile_value(lisp, c, clause_f);
        pk_u8 jump_from_t = pk_cmp_addr(c) + 1 - patch_2;

        pk_cmp_push_byte(lisp, c, PK_OP_BLOCK_END);

        pk_cmp_patch_byte(lisp, c, patch_1, jump_to_f);
        pk_cmp_patch_byte(lisp, c, patch_2, jump_from_t);

        *is_special = true;
    } else if (symbol == lisp->cache.quasiquote) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);
        (void)pk_atom_assert_nil(lisp, a->cdr);
        pk_compile_quasiquote(lisp, c, a->car);

        *is_special = true;
    } else if (symbol == lisp->cache.unquote) {
        pk_errors(lisp, pk_str("unquote outside quasiquote"));
    } else if (symbol == lisp->cache.let) {
        pk_compile_let(lisp, c, args, PK_ENV_VAR);

        *is_special = true;
    } else if (symbol == lisp->cache.flet) {
        pk_compile_let(lisp, c, args, PK_ENV_FUN);

        *is_special = true;
    } else if (symbol == lisp->cache.let_star) {
        pk_compile_let_star(lisp, c, args, PK_ENV_VAR);

        *is_special = true;
    } else if (symbol == lisp->cache.flet_star) {
        pk_compile_let_star(lisp, c, args, PK_ENV_FUN);

        *is_special = true;
    } else if (symbol == lisp->cache.progn) {
        pk_compile_progn(lisp, c, args);

        *is_special = true;
    } else if (symbol == lisp->cache.string_substitute) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);
        PKAtomString *string = pk_atom_assert_string(lisp, a->car);
        (void)pk_atom_assert_nil(lisp, a->cdr);
        pk_compile_strsub(lisp, c, string->str);

        *is_special = true;
    } else if (symbol == lisp->cache.var) {
        pk_compile_var(lisp, c, args, PK_ENV_VAR);

        *is_special = true;
    } else if (symbol == lisp->cache.protect) {
        pk_compile_protect(lisp, c, args);

        *is_special = true;
    } else if (symbol == lisp->cache.fvar) {
        pk_compile_var(lisp, c, args, PK_ENV_FUN);

        *is_special = true;
    } else if (symbol == lisp->cache.sym_and) {
        pk_compile_jmp(lisp, c, args, PK_OP_JMP_UNLESS, PK_OP_LOAD_TRUE);

        *is_special = true;
    } else if (symbol == lisp->cache.sym_or) {
        pk_compile_jmp(lisp, c, args, PK_OP_JMP_WHEN, PK_OP_LOAD_NIL);

        *is_special = true;
    } else if (symbol == lisp->cache.setq) {
        pk_compile_set(lisp, c, args, PK_ENV_VAR);

        *is_special = true;
    } else if (symbol == lisp->cache.fsetq) {
        pk_compile_set(lisp, c, args, PK_ENV_FUN);

        *is_special = true;
    } else if (symbol == lisp->cache.defclass) {
        pk_compile_defclass(lisp, c, args);
        
        *is_special = true;
    } else {
        *is_special = false;
    }
}

void pk_compile_evlist(Pocket lisp, PKCompiler *c, PKAtom *args) {
    PKAtom *iter = args;

    while (pk_atom_is_cons(iter)) {
        pk_compile_value(lisp, c, iter->cons.car);
        iter = iter->cons.cdr;
    }
}

void pk_compile_expression(Pocket lisp, PKCompiler *c, PKAtom *expr) {
    PKAtomCons *a = pk_atom_assert_cons(lisp, expr);
    PKAtomSymbol *symbol = pk_atom_assert_symbol(lisp, a->car);
    PKAtom *body = a->cdr;

    {
        bool is_special = false;
        pk_compile_special(lisp, c, symbol, body, &is_special);

        if (is_special) {
            return;
        }
    }

    size_t acc = 0;
    PKAtom *iter = body;
    while (pk_atom_is_cons(iter)) {
        pk_compile_value(lisp, c, iter->cons.car);
        acc += 1;
        iter = iter->cons.cdr;
    }

    if (acc >= UINT8_MAX) {
        pk_errors(lisp, pk_str("too many expressions"));
    }

    pk_cmp_load(lisp, c, (PKAtom *)symbol);

    pk_cmp_push_byte(lisp, c, PK_OP_CALL);
    pk_cmp_push_byte(lisp, c, (pk_u8)acc);
}

void pk_compile_value(Pocket lisp, PKCompiler *c, PKAtom *value) {
    switch (value->header.type) {
        case PK_ATOMTY_CONS: {
            pk_compile_expression(lisp, c, value);
            break;
        }
        case PK_ATOMTY_SYMBOL: {
            pk_cmp_lookup(lisp, c, (PKAtomSymbol *)value, PK_ENV_VAR);
            break;
        }
        default: {
            pk_cmp_load(lisp, c, value);
            break;
        }
    }
}

PKAtomLFun *pk_compile_compile(Pocket lisp, PKCompiler *c, pk_u8 args, pk_u8 arity, pk_u8 options) {
    PKAtomLFun *result = (PKAtomLFun *)pk_atom_alloc(lisp);

    for (size_t i = c->atoms.count; i < c->atoms.capacity; ++i) {
        c->atoms.e[i] = lisp->cache.nil;
    }

    for (size_t i = c->bc.count; i < c->bc.capacity; ++i) {
        c->bc.e[i] = PK_OP_ILLEGAL;
    }

    if (c->type == PK_CALL_FUN) {
        result->header.type = PK_ATOMTY_LFUN;
    } else {
        result->header.type = PK_ATOMTY_LMACRO;
    }
    result->atoms.e = c->atoms.e;
    result->atoms.length = c->atoms.capacity;
    result->bc.e = c->bc.e;
    result->bc.length = c->bc.capacity;
    result->arity = (PKArity) {
        .type = arity,
        .args = args,
        .options = options,
    };

    c->transferred = true;

    return result;
}

PKAtomLFun *pk_compile_lambda_(Pocket lisp, PKCompiler *c, PKAtom *args, PKAtom *body) {
    pk_u8 nargs = 0;
    pk_u8 arity = PK_INT_ARITY_NORMAL;
    pk_u8 option_start = 0;
    bool optional = false;
    bool variadic = false;
    bool patterns = false;

    {
        PKAtom *iter = args;
        while (pk_atom_is_cons(iter)) {
            PKAtom *atom = iter->cons.car;
            PKAtom *tail = iter->cons.cdr;

            if (pk_atom_is_keyword(atom)) {
                PKAtomKeyword *keyword = (PKAtomKeyword *)atom;
                if (variadic) {
                    pk_errors(lisp, pk_str("bad keyword arg"));
                }
                if (pk_atom_keyword_cmp(keyword, pk_str("rest"))) {
                    arity = PK_INT_ARITY_VARIADIC;
                    variadic = true;
                } else if (pk_atom_keyword_cmp(keyword, pk_str("opt")) ||
                           pk_atom_keyword_cmp(keyword, pk_str("optional"))) {
                    if (!optional) {
                        option_start = nargs;
                        optional = true;
                    }
                    arity = PK_INT_ARITY_OPTIONAL;

                    if ((!pk_atom_is_cons(tail)) || (!pk_atom_is_symbol(tail->cons.car))) {
                        pk_errors(lisp, pk_str("bad destructure"));
                    }
                }
            } else {
                if (pk_atom_is_cons(atom)) {
                    if (optional || variadic) {
                        pk_errors(lisp, pk_str("bad destructure"));
                    }
                    patterns = true;
                } else {
                    pk_atom_assert_symbol(lisp, atom);
                }

                if (variadic) {
                    (void)pk_atom_assert_nil(lisp, iter->cons.cdr);
                } else {
                    if (nargs == (UINT8_MAX - 1)) {
                        pk_errors(lisp, pk_str("too many args"));
                    }
                    nargs += 1;
                }
            }

            iter = iter->cons.cdr;
        }
    }

    if (!optional) {
        option_start = nargs;
    }

    pk_u8 lets = variadic ? (pk_u8)(nargs + 1) : nargs;

    if (patterns) {
        size_t pushed = 0;
        pk_u8 slots = lets;
        size_t index = 0;

        PKAtom *iter = args;
        while (pk_atom_is_cons(iter)) {
            PKAtom *atom = iter->cons.car;

            if (pk_atom_is_keyword(atom)) {
                iter = iter->cons.cdr;
                continue;
            }

            size_t bias = (slots - 1) - index;

            if (pk_atom_is_symbol(atom)) {
                pk_cmp_push_byte(lisp, c, PK_OP_DIG);
                pk_cmp_push_any(lisp, c, pushed + bias);
                pushed += 1;
            } else {
                pk_compile_pattern_values(lisp, c, atom, bias, false, &pushed);
            }

            index += 1;
            iter = iter->cons.cdr;
        }

        {
            size_t leaves = 0;

            iter = args;
            while (pk_atom_is_cons(iter)) {
                PKAtom *atom = iter->cons.car;

                if (pk_atom_is_keyword(atom)) {
                    iter = iter->cons.cdr;
                    continue;
                }

                if (pk_atom_is_symbol(atom)) {
                    pk_cmp_load(lisp, c, atom);
                    leaves += 1;
                } else {
                    pk_compile_pattern_symbols(lisp, c, atom, false, &leaves);
                }

                iter = iter->cons.cdr;
            }

            lets = (pk_u8)leaves;
        }
    } else {
        PKAtom *iter = args;
        while (pk_atom_is_cons(iter)) {
            PKAtom *atom = iter->cons.car;
            if ((!pk_atom_is_keyword(atom)) && pk_atom_is_symbol(atom)) {
                pk_cmp_load(lisp, c, atom);
            }
            iter = iter->cons.cdr;
        }
    }

    if (lets > 0) {
        pk_cmp_push_byte(lisp, c, PK_OP_LET_VAR);
        pk_cmp_push_byte(lisp, c, lets);
    }

    pk_compile_evlist(lisp, c, body);
    pk_cmp_push_byte(lisp, c, PK_OP_RET);

    return pk_compile_compile(lisp, c, nargs, arity, nargs - option_start);
}

typedef struct PKCompileLambdaData_ {
    PKCompiler c;
    PKAtom *args;
    PKAtom *body;
} PKCompileLambdaData;

void pk_compile_lambda_body(void *user_closure, Pocket lisp) {
    PKCompileLambdaData *data = (PKCompileLambdaData *)user_closure;
    PKAtomLFun *result = pk_compile_lambda_(lisp, &data->c, data->args, data->body);
    pk_stack_push(lisp, (PKAtom *)result);
}

PKAtomLFun *pk_compile_lambda(Pocket lisp, PK_CALL type, PKAtom *args, PKAtom *body) {
    PKCompileLambdaData data = (PKCompileLambdaData) {
        .c = pk_cmp_new(type),
        .args = args,
        .body = body,
    };

    PK_RES result = pk_protect(lisp, &data, pk_compile_lambda_body, 0);

    if (result == PK_YIELDED) {
        if (!data.c.transferred) {
            pk_cmp_deinit(lisp, &data.c);
        }
        pk_propagate(lisp);
    }

    PKAtom *out = pk_stack_head(lisp);
    pk_pop(lisp);
    return (PKAtomLFun *)out;
}

typedef struct PKAtomCompileData_ {
    PKCompiler c;
    PKAtom *value;
    PK_MODE mode;
    PKAtom *result;
} PKAtomCompileData;

void pk_atom_compile_body(void *user_closure, Pocket lisp) {
    PKAtomCompileData *data = (PKAtomCompileData *)user_closure;

    if (data->mode == PK_MODE_EXPRESSION) {
        pk_compile_value(lisp, &data->c, data->value);
    } else {
        pk_compile_progn(lisp, &data->c, data->value);
    }
    pk_cmp_push_byte(lisp, &data->c, PK_OP_RET);

    data->result = (PKAtom *)pk_compile_compile(lisp, &data->c, 0, PK_INT_ARITY_NORMAL, 0);
    pk_stack_push(lisp, data->result);
}

PKAtom *pk_atom_compile(Pocket lisp, PK_CALL type, PKAtom *value, PK_MODE mode) {
    PKAtomCompileData data = (PKAtomCompileData) {
        .c = pk_cmp_new(type),
        .value = value,
        .mode = mode,
        .result = NULL,
    };

    PK_RES result = pk_protect_noreturn(lisp, &data, pk_atom_compile_body, 0);

    if (result == PK_YIELDED) {
        if (!data.c.transferred) {
            pk_cmp_deinit(lisp, &data.c);
        }
        pk_propagate(lisp);
    }

    return data.result;
}
