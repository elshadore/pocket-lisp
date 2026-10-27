#include "pocket_internals.h"

#define pk_vat(vm_) ((vm_)->op)

PKVM pk_vm_init(Pocket lisp, PKAtom *lfun, size_t save) {
    return (PKVM) {
        .lisp = lisp,
        .lfun = lfun,
        .save = save,
        .curr = 0,
        .op = lfun->lfun.bc.e[0],
        .opcode = PK_OP_ILLEGAL,
        .finished = false,
    };
}

void pk_vm_inc(PKVM *vm) {
    vm->curr += 1;
    if (vm->curr >= vm->lfun->lfun.bc.length) {
        vm->op = PK_OP_ILLEGAL;
        pk_errors(vm->lisp, pk_str("vm out of bounds"));
    }
    vm->op = vm->lfun->lfun.bc.e[vm->curr];
}

void pk_vm_jmp(PKVM *vm, pk_u8 jmp) {
    vm->curr += jmp;
    if (vm->curr >= vm->lfun->lfun.bc.length) {
        vm->op = PK_OP_ILLEGAL;
        pk_errors(vm->lisp, pk_str("vm out of bounds"));
    }
    vm->op = vm->lfun->lfun.bc.e[vm->curr];
}

void pk_vm_jmp_back(PKVM *vm, pk_u8 jmp) {
    if (jmp > vm->curr) {
        vm->op = PK_OP_ILLEGAL;
        pk_errors(vm->lisp, pk_str("vm out of bounds"));
    }

    vm->curr -= jmp;
    vm->op = vm->lfun->lfun.bc.e[vm->curr];
}

PKAtom *pk_vm_get_atom(PKVM *vm, pk_u8 byte) {
    if (byte >= vm->lfun->lfun.atoms.length) {
        pk_errors(vm->lisp, pk_str("bad atom index"));
        return vm->lisp->cache.nil;
    }
    return vm->lfun->lfun.atoms.e[byte];
}

void pk_vm_op_let(PKVM *vm, PK_ENV env) {
    Pocket lisp = vm->lisp;

    pk_vm_inc(vm);

    size_t let = (size_t)pk_vat(vm);
    PKAtoms slice = pk_stack_slice_down(lisp, let * 2);

    for (size_t i = 0; i < let; ++i) {
        PKAtomSymbol *symbol = pk_atom_assert_symbol(lisp, slice.e[i + let]);
        (void)pk_env_let(lisp, env, symbol, slice.e[i]);
    }

    pk_stack_pop_unchecked(lisp, let * 2);

    pk_vm_inc(vm);
}

void pk_vm_op_get(PKVM *vm, PK_ENV env) {
    Pocket lisp = vm->lisp;

    pk_vm_inc(vm);

    PKAtomSymbol *symbol = pk_atom_assert_symbol(lisp, pk_vm_get_atom(vm, pk_vat(vm)));
    pk_stack_push(lisp, pk_env_get_strict(lisp, env, symbol));

    pk_vm_inc(vm);
}

void pk_vm_op_set(PKVM *vm, PK_ENV env) {
    Pocket lisp = vm->lisp;

    pk_vm_inc(vm);

    PKAtoms slice = pk_stack_slice_down(lisp, 2);

    PKAtomSymbol *symbol = pk_atom_assert_symbol(lisp, slice.e[0]);
    (void)pk_env_set(lisp, env, symbol, slice.e[1]);

    pk_stack_pop_unchecked(lisp, 1);
    lisp->stack.e[lisp->stack.count - 1] = slice.e[1];
}

void pk_vm_op_exec(PKVM *vm) {
    Pocket lisp = vm->lisp;

    vm->opcode = vm->op;
    switch (vm->op) {
        case PK_OP_ILLEGAL: {
            pk_errors(lisp, pk_str("illegal op"));
            return;
        }
        case PK_OP_NOP: {
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_LOAD: {
            pk_vm_inc(vm);
            // So we cannot mutate the data stored in the bytecode.
            PKAtom *new = pk_atom_clone(lisp, pk_vm_get_atom(vm, pk_vat(vm)));
            pk_stack_push(lisp, new);
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_LOAD_NIL: {
            pk_push_nil(lisp);
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_LOAD_TRUE: {
            pk_push_true(lisp);
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_CALL: {
            pk_vm_inc(vm);
            (void)pk_funcall(lisp, PK_CALL_FUN, pk_vat(vm));
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_BLOCK_BEGIN: {
            pk_frame_push(lisp, 0);
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_BLOCK_END: {
            pk_frame_return_push(lisp);
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_BLOCK_CLEAR: {
            pk_frame_clear(lisp);
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_BLOCK_POP: {
            pk_frame_unwind(lisp);
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_JMP_UNLESS: {
            pk_vm_inc(vm);
            PKAtom *atom = pk_stack_head(lisp);
            if (pk_atom_is_nil(atom)) {
                pk_vm_jmp(vm, pk_vat(vm));
            } else {
                pk_vm_inc(vm);
            }
            return;
        }
        case PK_OP_JMP_WHEN: {
            pk_vm_inc(vm);
            PKAtom *atom = pk_stack_head(lisp);
            if (!pk_atom_is_nil(atom)) {
                pk_vm_jmp(vm, pk_vat(vm));
            } else {
                pk_vm_inc(vm);
            }
            return;
        }
        case PK_OP_JMP: {
            pk_vm_inc(vm);
            pk_vm_jmp(vm, pk_vat(vm));
            return;
        }
        case PK_OP_JMP_BACK: {
            pk_vm_inc(vm);
            pk_vm_jmp_back(vm, pk_vat(vm));
            return;
        }
        case PK_OP_LET_VAR: {
            pk_vm_op_let(vm, PK_ENV_VAR);
            return;
        }
        case PK_OP_LET_FUN: {
            pk_vm_op_let(vm, PK_ENV_FUN);
            return;
        }
        case PK_OP_LET_PROP: {
            pk_vm_op_let(vm, PK_ENV_PROP);
            return;
        }
        case PK_OP_GET_VAR: {
            pk_vm_op_get(vm, PK_ENV_VAR);
            return;
        }
        case PK_OP_GET_FUN: {
            pk_vm_op_get(vm, PK_ENV_FUN);
            return;
        }
        case PK_OP_GET_PROP: {
            pk_vm_op_get(vm, PK_ENV_PROP);
            return;
        }
        case PK_OP_SET_VAR: {
            pk_vm_op_set(vm, PK_ENV_VAR);
            return;
        }
        case PK_OP_SET_FUN: {
            pk_vm_op_set(vm, PK_ENV_FUN);
            return;
        }
        case PK_OP_SET_PROP: {
            pk_vm_op_set(vm, PK_ENV_PROP);
            return;
        }
        case PK_OP_MAKE_LIST: {
            pk_vm_inc(vm);
            pk_stack_op_list(lisp, pk_vat(vm));
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_MAKE_LIST_PACKED: {
            pk_vm_inc(vm);
            pk_stack_op_list_packed(lisp, pk_vat(vm));
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_MERGE_LISTS: {
            pk_vm_inc(vm);
            pk_stack_op_list_merge(lisp, pk_vat(vm));
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_STRCAT: {
            pk_vm_inc(vm);
            pk_stack_op_strcat(lisp, pk_vat(vm));
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_DIG: {
            pk_vm_inc(vm);
            pk_stack_op_dig(lisp, pk_vat(vm));
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_CAR: {
            pk_vm_inc(vm);
            pk_stack_op_car(lisp);
            return;
        }
        case PK_OP_CAR_STRICT: {
            pk_vm_inc(vm);
            pk_stack_op_car_strict(lisp);
            return;
        }
        case PK_OP_CDR: {
            pk_vm_inc(vm);
            pk_stack_op_cdr(lisp);
            return;
        }
        case PK_OP_CDR_STRICT: {
            pk_vm_inc(vm);
            pk_stack_op_cdr_strict(lisp);
            return;
        }
        case PK_OP_PROTECT_BEGIN: {
            pk_frame_push_mode(lisp, 0, PK_FRAME_MODE_PROTECT);
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_PROTECT_END: {
            pk_frame_return_push(lisp);
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_RET: {
            vm->finished = true;
            return;
        }
        case PK_OP_POP: {
            pk_pop(lisp);
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_POPN: {
            pk_vm_inc(vm);
            size_t n = (size_t)pk_vat(vm);
            pk_popn(lisp, n);
            pk_vm_inc(vm);
            return;
        }
        case PK_OP_DEFCLASS: {
            pk_errors(lisp, pk_str("TODO: implement this opcode"));
            return;
        }
        default: {
            pk_errors(lisp, pk_str("illegal op"));
            return;
        }
    }
}

void pk_vm_exec(Pocket lisp, PKVM *vm) {
    vm->lisp = lisp;

    while (!vm->finished) {
        pk_vm_op_exec(vm);
    }
}
