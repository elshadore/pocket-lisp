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

PKCompiler pk_cmp_new(Pocket lisp, PK_CALL type) {
    return (PKCompiler) {
        .lisp = lisp,
        .atoms = PK_ARRAY_EMPTY,
        .bc = PK_BYTES_EMPTY,
        .type = type,
        .addr = 0,
        .transferred = false,
    };
}

#define pk_cmp_addr(c_) ((c_)->addr)

void pk_cmp_patch_byte(PKCompiler *c, pk_u8 addr, pk_u8 byte) {
    if (addr >= c->bc.count) {
        pk_errors(c->lisp, pk_str("bad byte address"));
    }
    c->bc.e[addr] = byte;
}

void pk_cmp_push_byte(PKCompiler *c, pk_u8 byte) {
    if (c->bc.count >= UINT8_MAX) {
        pk_errors(c->lisp, pk_str("bytecode too big"));
    }

    c->addr = (pk_u8)c->bc.count;

    pk_bytes_push(c->lisp, &c->bc, byte, 8);
}

void pk_cmp_push_any(PKCompiler *c, size_t value) {
    if (value > UINT8_MAX) {
        pk_errors(c->lisp, pk_str("address too big"));
    }

    pk_cmp_push_byte(c, (pk_u8)value);
}

void pk_cmp_push_atom(PKCompiler *c, PKAtom *atom) {
    if (c->atoms.count >= UINT8_MAX) {
        pk_errors(c->lisp, pk_str("too many atoms"));
    }

    for (size_t i = 0; i < c->atoms.count; ++i) {
        PKAtom *cmp = c->atoms.e[i];
        if (pk_atom_eq(c->lisp, atom, cmp)) {
            c->addr = (pk_u8)i;
            return;
        }
    }

    c->addr = (pk_u8)c->atoms.count;
    pk_array_push(c->lisp, &c->atoms, atom, 8);
}

void pk_cmp_load_nil(PKCompiler *c) {
    pk_cmp_push_byte(c, PK_OP_LOAD_NIL);
}

void pk_cmp_load_true(PKCompiler *c) {
    pk_cmp_push_byte(c, PK_OP_LOAD_TRUE);
}

void pk_cmp_load(PKCompiler *c, PKAtom *atom) {
    if (pk_atom_is_nil(atom)) {
        pk_cmp_load_nil(c);
        return;
    }
    if (atom->header.type == PK_ATOMTY_TRUE) {
        pk_cmp_load_true(c);
        return;
    }

    pk_cmp_push_atom(c, atom);
    pk_u8 addr = pk_cmp_addr(c);

    pk_cmp_push_byte(c, PK_OP_LOAD);
    pk_cmp_push_byte(c, addr);
}

void pk_cmp_lookup(PKCompiler *c, PKAtomSymbol *symbol, PK_ENV env) {
    pk_cmp_push_atom(c, (PKAtom *)symbol);
    pk_u8 addr = pk_cmp_addr(c);

    switch (env) {
        case PK_ENV_VAR: {
            pk_cmp_push_byte(c, PK_OP_GET_VAR);
            break;
        }
        case PK_ENV_FUN: {
            pk_cmp_push_byte(c, PK_OP_GET_FUN);
            break;
        }
        case PK_ENV_PROP: {
            pk_cmp_push_byte(c, PK_OP_GET_PROP);
            break;
        }
    }

    pk_cmp_push_byte(c, addr);
}

void pk_cmp_deinit(PKCompiler *c) {
    if (!c->transferred) {
        pk_free(c->lisp, c->atoms.e);
        pk_free(c->lisp, c->bc.e);
    }
    PK_CALL type = c->type;
    *c = pk_cmp_new(c->lisp, type);
}

void pk_compile_evlist(PKCompiler *c, PKAtom *args);

void pk_compile_value(PKCompiler *c, PKAtom *value);
PKAtomLFun *pk_compile_lambda_(PKCompiler *c, PKAtom *args, PKAtom *body);
PKAtomLFun *pk_compile_lambda(Pocket lisp, PK_CALL type, PKAtom *args, PKAtom *body);
void pk_compile_strsub(PKCompiler *c, PKStr string);

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

void pk_compile_pattern_path(PKCompiler *c, pk_u8 *path, size_t length, bool optional) {
    for (size_t i = 0; i < length; ++i) {
        if (path[i] == PK_PATTERN_PATH_CAR) {
            pk_cmp_push_byte(c, pk_pattern_car_op(optional));
        } else {
            pk_cmp_push_byte(c, pk_pattern_cdr_op(optional));
        }
    }
}

void pk_compile_pattern_cdrs(PKCompiler *c, size_t index, bool optional) {
    pk_u8 op = pk_pattern_cdr_op(optional);
    for (size_t i = 0; i < index; ++i) {
        pk_cmp_push_byte(c, op);
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

void pk_compile_pattern_values_(PKCompiler *c, PKAtom *pattern, pk_u8 *path, size_t length, size_t base, size_t bias, size_t *pushed, bool optional) {
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
                    pk_errors(c->lisp, pk_str("bad destructure"));
                }
            } else if (pk_atom_keyword_cmp((PKAtomKeyword *)atom, pk_str("rest"))) {
                if ((!pk_atom_is_cons(tail)) || (!pk_atom_is_symbol(tail->cons.car)) || (!pk_atom_is_nil(tail->cons.cdr))) {
                    pk_errors(c->lisp, pk_str("bad destructure"));
                }

                pk_cmp_push_byte(c, PK_OP_DIG);
                pk_cmp_push_any(c, (*pushed - base) + bias);
                pk_compile_pattern_path(c, path, length, optional);
                pk_compile_pattern_cdrs(c, index, true);
                *pushed += 1;
                return;
            } else {
                pk_errors(c->lisp, pk_str("bad destructure"));
            }
        } else if (pk_atom_is_symbol(atom)) {
            pk_cmp_push_byte(c, PK_OP_DIG);
            pk_cmp_push_any(c, (*pushed - base) + bias);
            pk_compile_pattern_path(c, path, length, optional);
            pk_compile_pattern_cdrs(c, index, optional);
            pk_cmp_push_byte(c, pk_pattern_car_op(optional));
            *pushed += 1;
        } else if (pk_atom_is_cons(atom)) {
            if (optional) {
                pk_errors(c->lisp, pk_str("bad destructure"));
            }
            pk_compile_pattern_descend(c->lisp, path, length, index);
            pk_compile_pattern_values_(c, atom, path, length + index + 1, base, bias, pushed, optional);
        } else {
            pk_errors(c->lisp, pk_str("bad destructure"));
        }

        iter = tail;
        if (!pk_atom_is_keyword(atom)) {
            index += 1;
        }
    }

    if (!pk_atom_is_nil(iter)) {
        pk_errors(c->lisp, pk_str("bad destructure"));
    }
}

void pk_compile_pattern_values(PKCompiler *c, PKAtom *pattern, size_t bias, bool optional, size_t *pushed) {
    pk_u8 path[PK_PATTERN_PATH_MAX] = {0};
    pk_compile_pattern_values_(c, pattern, path, 0, 0, bias, pushed, optional);
}

void pk_compile_pattern_symbols_(PKCompiler *c, PKAtom *pattern, bool optional, size_t *leaves) {
    PKAtom *iter = pattern;
    while (pk_atom_is_cons(iter)) {
        PKAtom *atom = iter->cons.car;
        PKAtom *tail = iter->cons.cdr;

        if (pk_atom_is_keyword(atom)) {
            if ((pk_atom_keyword_cmp((PKAtomKeyword *)atom, pk_str("opt"))) ||
                (pk_atom_keyword_cmp((PKAtomKeyword *)atom, pk_str("optional")))) {
                optional = true;
                if ((!pk_atom_is_cons(tail)) || (!pk_atom_is_symbol(tail->cons.car))) {
                    pk_errors(c->lisp, pk_str("bad destructure"));
                }
            } else if (pk_atom_keyword_cmp((PKAtomKeyword *)atom, pk_str("rest"))) {
                if ((!pk_atom_is_cons(tail)) || (!pk_atom_is_symbol(tail->cons.car)) || (!pk_atom_is_nil(tail->cons.cdr))) {
                    pk_errors(c->lisp, pk_str("bad destructure"));
                }
                pk_cmp_load(c, tail->cons.car);
                *leaves += 1;
                return;
            } else {
                pk_errors(c->lisp, pk_str("bad destructure"));
            }
        } else if (pk_atom_is_symbol(atom)) {
            pk_cmp_load(c, atom);
            *leaves += 1;
        } else if (pk_atom_is_cons(atom)) {
            if (optional) {
                pk_errors(c->lisp, pk_str("bad destructure"));
            }
            pk_compile_pattern_symbols_(c, atom, optional, leaves);
        } else {
            pk_errors(c->lisp, pk_str("bad destructure"));
        }

        iter = tail;
    }

    if (!pk_atom_is_nil(iter)) {
        pk_errors(c->lisp, pk_str("bad destructure"));
    }
}

void pk_compile_pattern_symbols(PKCompiler *c, PKAtom *pattern, bool optional, size_t *leaves) {
    pk_compile_pattern_symbols_(c, pattern, optional, leaves);
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

void pk_compile_let_lets(PKCompiler *c, PK_ENV env) {
    switch (env) {
        case PK_ENV_VAR: {
            pk_cmp_push_byte(c, PK_OP_LET_VAR);
            break;
        }
        case PK_ENV_FUN: {
            pk_cmp_push_byte(c, PK_OP_LET_FUN);
            break;
        }
        case PK_ENV_PROP: {
            pk_cmp_push_byte(c, PK_OP_LET_PROP);
            break;
        }
    }
}

void pk_compile_bind(PKCompiler *c, PKAtom *bind, PK_ENV env) {
    switch (bind->header.type) {
        case PK_ATOMTY_CONS: {
            size_t pushed = 0;
            pk_compile_pattern_values(c, bind, 0, false, &pushed);

            size_t lets = 0;
            pk_compile_pattern_symbols(c, bind, false, &lets);

            pk_compile_let_lets(c, env);
            pk_cmp_push_any(c, lets);
            break;
        }
        case PK_ATOMTY_NIL: break;
        default: {
            pk_error_type(c->lisp, PK_TYPE_LIST, bind);
            break;
        }
    }
}

void pk_compile_let_binding_value(PKCompiler *c, PKAtom *binding) {
    if (pk_atom_is_symbol(binding)) {
        pk_cmp_load_nil(c);
        return;
    }

    PKAtomCons *a = pk_atom_assert_cons(c->lisp, binding);
    PKAtomCons *b = pk_atom_assert_cons(c->lisp, a->cdr);
    if (!pk_atom_is_nil(b->cdr)) {
        pk_errors(c->lisp, pk_str("bad body"));
    }
    pk_compile_value(c, b->car);
}

void pk_compile_let(PKCompiler *c, PKAtom *args, PK_ENV env) {
    PKAtomCons *a = pk_atom_assert_cons(c->lisp, args);

    PKAtom *bindings = a->car;
    PKAtom *body = a->cdr;

    pk_cmp_push_byte(c, PK_OP_BLOCK_BEGIN);

    bool patterns = pk_compile_let_has_patterns(bindings);

    size_t values = 0;
    {
        PKAtom *iter = bindings;
        while (pk_atom_is_cons(iter)) {
            pk_compile_let_binding_value(c, iter->cons.car);
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
                pk_compile_pattern_values(c, binding->cons.car, (values - 1) - index, false, &pushed);
            } else {
                pk_cmp_push_byte(c, PK_OP_DIG);
                pk_cmp_push_any(c, pushed + ((values - 1) - index));
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
                pk_compile_pattern_symbols(c, binding->cons.car, false, &lets);
            } else {
                PKAtom *symbol = c->lisp->cache.nil;
                if (pk_atom_is_symbol(binding)) {
                    symbol = binding;
                } else {
                    symbol = (PKAtom *)pk_atom_assert_symbol(c->lisp, pk_atom_assert_cons(c->lisp, binding)->car);
                }
                pk_cmp_load(c, symbol);
                lets += 1;
            }

            iter = iter->cons.cdr;
        }
    }

    pk_compile_let_lets(c, env);
    pk_cmp_push_any(c, lets);
    pk_compile_evlist(c, body);
    pk_cmp_push_byte(c, PK_OP_BLOCK_END);
}

void pk_compile_let_star(PKCompiler *c, PKAtom *args, PK_ENV env) {
    if (!pk_atom_is_cons(args)) {
        pk_errors(c->lisp, pk_str("bad let args"));
    }

    PKAtomCons *a = pk_atom_assert_cons(c->lisp, args);

    PKAtom *bindings = a->car;
    PKAtom *body = a->cdr;

    pk_cmp_push_byte(c, PK_OP_BLOCK_BEGIN);

    PKAtom *iter = bindings;
    while (pk_atom_is_cons(iter)) {
        PKAtom *binding = iter->cons.car;

        if (pk_let_binding_is_pattern(binding)) {
            PKAtom *pa = binding->cons.car;
            PKAtomCons *b = pk_atom_assert_cons(c->lisp, binding->cons.cdr);
            if (!pk_atom_is_nil(b->cdr)) {
                pk_errors(c->lisp, pk_str("bad let binding"));
            }

            pk_compile_value(c, b->car);

            size_t pushed = 0;
            pk_compile_pattern_values(c, pa, 0, false, &pushed);

            size_t lets = 0;
            pk_compile_pattern_symbols(c, pa, false, &lets);

            pk_compile_let_lets(c, env);
            pk_cmp_push_any(c, lets);

            iter = iter->cons.cdr;
            continue;
        }

        PKAtom *symbol = c->lisp->cache.nil;
        if (pk_atom_is_symbol(binding)) {
            pk_cmp_load_nil(c);
            symbol = binding;
        } else {
            PKAtomCons *ba = pk_atom_assert_cons(c->lisp, binding);
            PKAtomCons *b = pk_atom_assert_cons(c->lisp, ba->cdr);
            if (!pk_atom_is_nil(b->cdr)) {
                pk_errors(c->lisp, pk_str("bad let binding"));
            }
            symbol = (PKAtom *)pk_atom_assert_symbol(c->lisp, ba->car);
            pk_compile_value(c, b->car);
        }

        pk_cmp_load(c, symbol);
        pk_compile_let_lets(c, env);
        pk_cmp_push_byte(c, 1);

        iter = iter->cons.cdr;
    }

    pk_compile_evlist(c, body);
    pk_cmp_push_byte(c, PK_OP_BLOCK_END);
}

void pk_compile_quasiquote(PKCompiler *c, PKAtom *value);
void pk_compile_quasiquote_list(PKCompiler *c, PKAtom *list);
void pk_compile_quasiquote_value(PKCompiler *c, PKAtom *value, PKAtom **spliced);

void pk_compile_quasiquote_value(PKCompiler *c, PKAtom *value, PKAtom **spliced) {
    if (!pk_atom_is_cons(value)) {
        pk_cmp_load(c, value);
        return;
    }

    PKAtom *a = value->cons.car;

    if (pk_atom_is_symbol(a)) {
        PKAtomSymbol *asymbol = (PKAtomSymbol *)a;

        if (asymbol == c->lisp->cache.quote) {
            PKAtomCons *quoted = pk_atom_assert_cons(c->lisp, value->cons.cdr);
            (void)pk_atom_assert_nil(c->lisp, quoted->cdr);
            pk_cmp_load(c, (PKAtom *)c->lisp->cache.quote);
            pk_compile_quasiquote_value(c, quoted->car, spliced);
            pk_cmp_push_byte(c, PK_OP_MAKE_LIST);
            pk_cmp_push_byte(c, 2);
            return;
        }

        if (asymbol == c->lisp->cache.unquote) {
            PKAtom *b = value->cons.cdr;
            if (!pk_atom_is_cons(b)) {
                pk_errors(c->lisp, pk_str("bad unquote"));
            }
            if (!pk_atom_is_nil(b->cons.cdr)) {
                pk_errors(c->lisp, pk_str("bad unquote"));
            }
            pk_compile_value(c, b->cons.car);
            return;
        } else if (asymbol == c->lisp->cache.unquote_splice) {
            PKAtom *b = value->cons.cdr;
            if (!pk_atom_is_cons(b)) {
                pk_errors(c->lisp, pk_str("bad unquote splice"));
            }
            if (!pk_atom_is_nil(b->cons.cdr)) {
                pk_errors(c->lisp, pk_str("bad unquote splice"));
            }
            *spliced = b->cons.car;
            return;
        }
    }
    pk_compile_quasiquote_list(c, value);
}

void pk_compile_qqlist(PKCompiler *c, PKQQ *qq) {
    if (qq->packed_cons) {
        pk_cmp_push_byte(c, PK_OP_MAKE_LIST_PACKED);
    } else {
        pk_cmp_push_byte(c, PK_OP_MAKE_LIST);
    }
    pk_cmp_push_any(c, qq->count);

    qq->packed_cons = false;
    qq->count = 0;
    qq->spliced_count += 1;
}

void pk_compile_quasiquote_list(PKCompiler *c, PKAtom *list) {
    PKQQ qq = PK_QQ_EMPTY;

    PKAtom *iter = list;

    while (!pk_atom_is_nil(iter)) {

        PKAtom *spliced = NULL;

        if (pk_atom_is_cons(iter)) {
            pk_compile_quasiquote_value(c, iter->cons.car, &spliced);
            iter = iter->cons.cdr;
        } else {
            pk_compile_quasiquote_value(c, iter, &spliced);
            qq.packed_cons = true;
            iter = c->lisp->cache.nil;
        }

        if (spliced != NULL) {
            qq.spliced = true;
            qq.spliced_last = true;
            pk_compile_qqlist(c, &qq);
            pk_compile_value(c, spliced);
        } else {
            qq.spliced_last = false;
            qq.count += 1;
        }
    }

    if (!qq.spliced_last) {
        pk_compile_qqlist(c, &qq);
    }

    if (qq.spliced) {
        pk_cmp_push_byte(c, PK_OP_MERGE_LISTS);
        pk_cmp_push_any(c, qq.spliced_count + 1);
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

void pk_compile_evlist_except_last(PKCompiler *c, PKAtom *args) {
    PKAtom *iter = args;
    while (pk_atom_is_cons(iter) && pk_atom_is_cons(iter->cons.cdr)) {
        pk_compile_value(c, iter->cons.car);
        iter = iter->cons.cdr;
    }
}

void pk_compile_set(PKCompiler *c, PKAtom *args, PK_ENV env) {
    PKAtomCons *a = pk_atom_assert_cons(c->lisp, args);
    PKAtomCons *b = pk_atom_assert_cons(c->lisp, a->cdr);
    (void)pk_atom_assert_nil(c->lisp, b->cdr);
    pk_atom_assert_symbol(c->lisp, a->car);

    pk_cmp_load(c, a->car);
    pk_compile_value(c, b->car);

    switch (env) {
        case PK_ENV_VAR: {
            pk_cmp_push_byte(c, PK_OP_SET_VAR);
            break;
        }
        case PK_ENV_FUN: {
            pk_cmp_push_byte(c, PK_OP_SET_FUN);
            break;
        }
        case PK_ENV_PROP: {
            pk_cmp_push_byte(c, PK_OP_SET_PROP);
            break;
        }
    }
}

void pk_compile_protect_evlist(PKCompiler *c, PKAtom *args) {
    PKAtom *iter = args;

    while (pk_atom_is_cons(iter)) {
        PKAtom *lfun = pk_atom_compile(c->lisp, PK_CALL_FUN, iter->cons.car, PK_MODE_EXPRESSION);
        pk_cmp_load(c, lfun);
        pk_cmp_push_byte(c, PK_OP_CALL);
        pk_cmp_push_byte(c, 0);
        iter = iter->cons.cdr;
    }
}

void pk_compile_protect(PKCompiler *c, PKAtom *args) {
    pk_cmp_push_byte(c, PK_OP_PROTECT_BEGIN);
    pk_compile_protect_evlist(c, args);
    pk_cmp_push_byte(c, PK_OP_PROTECT_END);
}

void pk_compile_quasiquote(PKCompiler *c, PKAtom *value) {
    PKAtom *spliced = NULL;
    pk_compile_quasiquote_value(c, value, &spliced);
    if (spliced != NULL) {
        pk_errors(c->lisp, pk_str("bad quasiquote"));
    }
}

void pk_compile_jmp_rec_(PKCompiler *c, PKAtom *args, pk_u8 op, PKBytes *patches) {
    pk_compile_value(c, args->cons.car);

    if (pk_atom_is_nil(args->cons.cdr)) {
        return;
    }

    pk_cmp_push_byte(c, op);
    pk_cmp_push_byte(c, 0);
    pk_bytes_push(c->lisp, patches, pk_cmp_addr(c), 16);

    pk_compile_jmp_rec_(c, args->cons.cdr, op, patches);
}

void pk_compile_jmp_(PKCompiler *c, PKAtom *args, pk_u8 op, pk_u8 empty_op, PKBytes *patches) {
    if (!pk_atom_is_cons(args)) {
        if (pk_atom_is_nil(args)) {
            pk_cmp_push_byte(c, empty_op);
            return;
        }
        pk_error_type(c->lisp, PK_TYPE_SEQ, args);
        return;
    }

    pk_cmp_push_byte(c, PK_OP_BLOCK_BEGIN);
    pk_compile_jmp_rec_(c, args, op, patches);
    pk_cmp_push_byte(c, PK_OP_BLOCK_END);

    pk_u8 end = pk_cmp_addr(c);
    for (size_t i = 0; i < patches->count; ++i) {
        pk_cmp_patch_byte(c, patches->e[i], end - patches->e[i]);
    }
}

void pk_compile_jmp(PKCompiler *c, PKAtom *args, pk_u8 op, pk_u8 empty_op) {
    PKBytes patches = PK_BYTES_EMPTY;
    pk_compile_jmp_(c, args, op, empty_op, &patches);
    pk_free(c->lisp, patches.e);
}

void pk_compile_progn(PKCompiler *c, PKAtom *atom) {
    pk_cmp_push_byte(c, PK_OP_BLOCK_BEGIN);
    pk_compile_evlist(c, atom);
    pk_cmp_push_byte(c, PK_OP_BLOCK_END);
}

void pk_compile_var(PKCompiler *c, PKAtom *form, PK_ENV env) {
    PKAtomCons *a = pk_atom_assert_cons(c->lisp, form);
    PKAtomCons *b = pk_atom_assert_cons(c->lisp, a->cdr);
    (void)pk_atom_assert_nil(c->lisp, b->cdr);

    pk_compile_value(c, b->car);

    if (pk_atom_is_cons(a->car)) {
        pk_compile_bind(c, a->car, env);
    } else {
        pk_atom_assert_symbol(c->lisp, a->car);
        pk_cmp_load(c, a->car);

        switch (env) {
            case PK_ENV_VAR: {
                pk_cmp_push_byte(c, PK_OP_LET_VAR);
                pk_cmp_push_byte(c, 1);
                break;
            }
            case PK_ENV_FUN: {
                pk_cmp_push_byte(c, PK_OP_LET_FUN);
                pk_cmp_push_byte(c, 1);
                break;
            }
            case PK_ENV_PROP: {
                pk_cmp_push_byte(c, PK_OP_LET_PROP);
                pk_cmp_push_byte(c, 1);
                break;
            }
        }
    }
}

void pk_compile_special(PKCompiler *c, PKAtomSymbol *symbol, PKAtom *args, bool *is_special) {
    Pocket lisp = c->lisp;

    if (symbol == lisp->cache.lambda) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);
        PKAtomLFun *lfun = pk_compile_lambda(lisp, PK_CALL_FUN, a->car, a->cdr);
        pk_cmp_load(c, (PKAtom *)lfun);

        *is_special = true;
    } else if (symbol == lisp->cache.macro) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);
        PKAtomLFun *lfun = pk_compile_lambda(lisp, PK_CALL_MACRO, a->car, a->cdr);
        pk_cmp_load(c, (PKAtom *)lfun);

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

        pk_cmp_load(c, a->car);
        pk_cmp_load(c, (PKAtom *)lfun);
        pk_cmp_push_byte(c, PK_OP_SET_FUN);

        *is_special = true;
    } else if (symbol == lisp->cache.defvar) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);
        (void)pk_atom_assert_symbol(lisp, a->car);

        pk_cmp_load(c, a->car);

        if (pk_atom_is_cons(a->cdr)) {
            PKAtomCons *b = pk_atom_assert_cons(lisp, a->cdr);
            (void)pk_atom_assert_nil(lisp, b->cdr);
            pk_compile_value(c, b->car);
        } else {
            (void)pk_atom_assert_nil(lisp, a->cdr);
            pk_cmp_load_nil(c);
        }

        pk_cmp_push_byte(c, PK_OP_SET_VAR);

        *is_special = true;
    } else if (symbol == lisp->cache.quote) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);
        (void)pk_atom_assert_nil(lisp, a->cdr);
        pk_cmp_load(c, a->car);

        *is_special = true;
    } else if (symbol == lisp->cache.sym_while) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);

        PKAtom *predicate = a->car;
        PKAtom *body = a->cdr;

        pk_cmp_push_byte(c, PK_OP_BLOCK_BEGIN);
        pk_cmp_push_byte(c, PK_OP_BLOCK_CLEAR);

        pk_u8 start = pk_cmp_addr(c);

        pk_compile_value(c, predicate);
        pk_cmp_push_byte(c, PK_OP_JMP_UNLESS);
        pk_cmp_push_byte(c, 0);

        pk_u8 patch = pk_cmp_addr(c);

        pk_compile_evlist(c, body);
        pk_cmp_push_byte(c, PK_OP_JMP_BACK);
        pk_cmp_push_byte(c, pk_cmp_addr(c) - start + 1);
        pk_cmp_patch_byte(c, patch, pk_cmp_addr(c) + 1 - patch);

        pk_cmp_push_byte(c, PK_OP_BLOCK_END);

        *is_special = true;
    } else if (symbol == lisp->cache.sym_if) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);
        PKAtom *value = a->car;
        PKAtomCons *b = pk_atom_assert_cons(lisp, a->cdr);
        PKAtom *clause_t = b->car;
        PKAtomCons *d = pk_atom_assert_cons(lisp, b->cdr);
        PKAtom *clause_f = d->car;
        (void)pk_atom_assert_nil(lisp, d->cdr);

        pk_cmp_push_byte(c, PK_OP_BLOCK_BEGIN);
        pk_compile_value(c, value);

        pk_cmp_push_byte(c, PK_OP_JMP_UNLESS);
        pk_cmp_push_byte(c, 0);
        pk_u8 patch_1 = pk_cmp_addr(c);

        pk_compile_value(c, clause_t);
        pk_cmp_push_byte(c, PK_OP_JMP);
        pk_cmp_push_byte(c, 0);
        pk_u8 patch_2 = pk_cmp_addr(c);

        pk_u8 jump_to_f = patch_2 + 1 - patch_1;
        pk_compile_value(c, clause_f);
        pk_u8 jump_from_t = pk_cmp_addr(c) + 1 - patch_2;

        pk_cmp_push_byte(c, PK_OP_BLOCK_END);

        pk_cmp_patch_byte(c, patch_1, jump_to_f);
        pk_cmp_patch_byte(c, patch_2, jump_from_t);

        *is_special = true;
    } else if (symbol == lisp->cache.quasiquote) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);
        (void)pk_atom_assert_nil(lisp, a->cdr);
        pk_compile_quasiquote(c, a->car);

        *is_special = true;
    } else if (symbol == lisp->cache.unquote) {
        pk_errors(lisp, pk_str("unquote outside quasiquote"));
    } else if (symbol == lisp->cache.let) {
        pk_compile_let(c, args, PK_ENV_VAR);

        *is_special = true;
    } else if (symbol == lisp->cache.flet) {
        pk_compile_let(c, args, PK_ENV_FUN);

        *is_special = true;
    } else if (symbol == lisp->cache.let_star) {
        pk_compile_let_star(c, args, PK_ENV_VAR);

        *is_special = true;
    } else if (symbol == lisp->cache.flet_star) {
        pk_compile_let_star(c, args, PK_ENV_FUN);

        *is_special = true;
    } else if (symbol == lisp->cache.progn) {
        pk_compile_progn(c, args);

        *is_special = true;
    } else if (symbol == lisp->cache.string_substitute) {
        PKAtomCons *a = pk_atom_assert_cons(lisp, args);
        PKAtomString *string = pk_atom_assert_string(lisp, a->car);
        (void)pk_atom_assert_nil(lisp, a->cdr);
        pk_compile_strsub(c, string->str);

        *is_special = true;
    } else if (symbol == lisp->cache.var) {
        pk_compile_var(c, args, PK_ENV_VAR);

        *is_special = true;
    } else if (symbol == lisp->cache.protect) {
        pk_compile_protect(c, args);

        *is_special = true;
    } else if (symbol == lisp->cache.fvar) {
        pk_compile_var(c, args, PK_ENV_FUN);

        *is_special = true;
    } else if (symbol == lisp->cache.sym_and) {
        pk_compile_jmp(c, args, PK_OP_JMP_UNLESS, PK_OP_LOAD_TRUE);

        *is_special = true;
    } else if (symbol == lisp->cache.sym_or) {
        pk_compile_jmp(c, args, PK_OP_JMP_WHEN, PK_OP_LOAD_NIL);

        *is_special = true;
    } else if (symbol == lisp->cache.setq) {
        pk_compile_set(c, args, PK_ENV_VAR);

        *is_special = true;
    } else if (symbol == lisp->cache.fsetq) {
        pk_compile_set(c, args, PK_ENV_FUN);

        *is_special = true;
    } else if (symbol == lisp->cache.psetq) {
        pk_compile_set(c, args, PK_ENV_PROP);

        *is_special = true;
    } else {
        *is_special = false;
    }
}

void pk_compile_evlist(PKCompiler *c, PKAtom *args) {
    PKAtom *iter = args;

    while (pk_atom_is_cons(iter)) {
        pk_compile_value(c, iter->cons.car);
        iter = iter->cons.cdr;
    }
}

void pk_compile_expression(PKCompiler *c, PKAtom *expr) {
    PKAtomCons *a = pk_atom_assert_cons(c->lisp, expr);
    PKAtomSymbol *symbol = pk_atom_assert_symbol(c->lisp, a->car);
    PKAtom *body = a->cdr;

    {
        bool is_special = false;
        pk_compile_special(c, symbol, body, &is_special);

        if (is_special) {
            return;
        }
    }

    pk_cmp_load(c, (PKAtom *)symbol);

    size_t acc = 0;
    PKAtom *iter = body;
    while (pk_atom_is_cons(iter)) {
        pk_compile_value(c, iter->cons.car);
        acc += 1;
        iter = iter->cons.cdr;
    }

    if (acc >= UINT8_MAX) {
        pk_errors(c->lisp, pk_str("too many expressions"));
    }

    pk_cmp_push_byte(c, PK_OP_CALL);
    pk_cmp_push_byte(c, (pk_u8)acc);
}

void pk_compile_value(PKCompiler *c, PKAtom *value) {
    switch (value->header.type) {
        case PK_ATOMTY_CONS: {
            pk_compile_expression(c, value);
            break;
        }
        case PK_ATOMTY_SYMBOL: {
            pk_cmp_lookup(c, (PKAtomSymbol *)value, PK_ENV_VAR);
            break;
        }
        default: {
            pk_cmp_load(c, value);
            break;
        }
    }
}

PKAtomLFun *pk_compile_compile(PKCompiler *c, pk_u8 args, pk_u8 arity, pk_u8 options) {
    PKAtomLFun *result = (PKAtomLFun *)pk_atom_alloc(c->lisp);

    for (size_t i = c->atoms.count; i < c->atoms.capacity; ++i) {
        c->atoms.e[i] = c->lisp->cache.nil;
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

PKAtomLFun *pk_compile_lambda_(PKCompiler *c, PKAtom *args, PKAtom *body) {
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
                    pk_errors(c->lisp, pk_str("bad keyword arg"));
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
                        pk_errors(c->lisp, pk_str("bad destructure"));
                    }
                }
            } else {
                if (pk_atom_is_cons(atom)) {
                    if (optional || variadic) {
                        pk_errors(c->lisp, pk_str("bad destructure"));
                    }
                    patterns = true;
                } else {
                    pk_atom_assert_symbol(c->lisp, atom);
                }

                if (variadic) {
                    (void)pk_atom_assert_nil(c->lisp, iter->cons.cdr);
                } else {
                    if (nargs == (UINT8_MAX - 1)) {
                        pk_errors(c->lisp, pk_str("too many args"));
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
                pk_cmp_push_byte(c, PK_OP_DIG);
                pk_cmp_push_any(c, pushed + bias);
                pushed += 1;
            } else {
                pk_compile_pattern_values(c, atom, bias, false, &pushed);
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
                    pk_cmp_load(c, atom);
                    leaves += 1;
                } else {
                    pk_compile_pattern_symbols(c, atom, false, &leaves);
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
                pk_cmp_load(c, atom);
            }
            iter = iter->cons.cdr;
        }
    }

    if (lets > 0) {
        pk_cmp_push_byte(c, PK_OP_LET_VAR);
        pk_cmp_push_byte(c, lets);
    }

    pk_compile_evlist(c, body);
    pk_cmp_push_byte(c, PK_OP_RET);

    return pk_compile_compile(c, nargs, arity, nargs - option_start);
}

PKAtomLFun *pk_compile_lambda(Pocket lisp, PK_CALL type, PKAtom *args, PKAtom *body) {
    PKCompiler c = pk_cmp_new(lisp, type);
    PKAtomLFun *result = pk_compile_lambda_(&c, args, body);
    pk_cmp_deinit(&c);
    return result;
}

PKAtom *pk_atom_compile(Pocket lisp, PK_CALL type, PKAtom *value, PK_MODE mode) {
    PKCompiler c = pk_cmp_new(lisp, type);

    if (mode == PK_MODE_EXPRESSION) {
        pk_compile_value(&c, value);
    } else {
        pk_compile_progn(&c, value);
    }
    pk_cmp_push_byte(&c, PK_OP_RET);

    return (PKAtom *)pk_compile_compile(&c, 0, PK_INT_ARITY_NORMAL, 0);
}
