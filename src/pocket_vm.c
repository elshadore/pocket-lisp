#include "pocket_internals.h"

#define pk_vat(vm_) ((vm_)->op)

PKVM pk_vm_init(PKAtom lfun, size_t save) {
    return (PKVM) {
        .lfun = lfun,
        .save = save,
        .curr = 0,
        .op = lfun->lfun.bc.e[0],
        .finished = false,
    };
}

PKAtom pk_vm_inc(PKVM *vm) {
    vm->curr += 1;
    if (vm->curr >= vm->lfun->lfun.bc.length) {
        vm->op = PK_OP_ILLEGAL;
        return pk_error();
    }
    vm->op = vm->lfun->lfun.bc.e[vm->curr];
    return pk_true();
}

PKAtom pk_vm_jmp(PKVM *vm, pk_u8 jmp) {
    vm->curr += jmp;
    if (vm->curr >= vm->lfun->lfun.bc.length) {
        vm->op = PK_OP_ILLEGAL;
        return pk_error();
    }
    vm->op = vm->lfun->lfun.bc.e[vm->curr];
    return pk_true();
}

PKAtom pk_vm_jmp_back(PKVM *vm, pk_u8 jmp) {
    if (jmp > vm->curr) {
        vm->op = PK_OP_ILLEGAL;
        return pk_error();
    }
    
    vm->curr -= jmp;
    vm->op = vm->lfun->lfun.bc.e[vm->curr];
    return pk_true();
}

PKAtom pk_vm_get_atom(PKVM *vm, pk_u8 byte) {
    if (byte >= vm->lfun->lfun.atoms.length) {
        return pk_error();
    }
    return vm->lfun->lfun.atoms.e[byte];
}

PKAtom pk_vm_op_let(PKVM *vm, PK_ENV env) {
    pk_propagate(pk_vm_inc(vm));
    
    PKAtoms slice = PK_ATOMS_EMPTY;
    size_t let = (size_t)pk_vat(vm);
    pk_propagate(pk_stack_slice_down(let * 2, &slice));

    for (size_t i = 0; i < let; ++i) {
        PKAtom symbol = slice.e[i + let];
        pk_propagate(pk_typecheck_symbol(symbol));
        pk_propagate(pk_let(env, symbol, slice.e[i]));
    }
    
    pk_propagate(pk_vm_inc(vm));
    return pk_true();
}

PKAtom pk_vm_op_get(PKVM *vm, PK_ENV env) {
    pk_propagate(pk_vm_inc(vm));
    
    PKAtom symbol = pk_vm_get_atom(vm, pk_vat(vm));
    pk_propagate(symbol);
    pk_propagate(pk_typecheck_symbol(symbol));
    PKAtom result = pk_get(env, symbol);
    pk_propagate(result);
    
    pk_propagate(pk_stack_push(result));
    pk_propagate(pk_vm_inc(vm));
    
    return pk_true();
}

PKAtom pk_vm_op_set(PKVM *vm, PK_ENV env) {
    pk_propagate(pk_vm_inc(vm));

    PKAtoms slice = PK_ATOMS_EMPTY;
    pk_propagate(pk_stack_slice_down(2, &slice));

    PKAtom symbol = slice.e[0];
    PKAtom value = slice.e[1];
    pk_propagate(pk_typecheck_symbol(symbol));
    pk_propagate(pk_set(env, symbol, value));

    PKAtoms popped = PK_ATOMS_EMPTY;
    pk_propagate(pk_stack_slice_down(2, &popped));
    pk_stack_pop_unchecked(1);
    PK_CORE.stack.e[PK_CORE.stack.count - 1] = value;
    pk_propagate(pk_vm_inc(vm));

    return pk_true();
}

PKAtom pk_vm_op_exec(PKVM *vm) {
    switch (pk_vat(vm)) {
        case PK_OP_ILLEGAL: {
            return pk_error();
        }
        case PK_OP_NOP: {
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_LOAD: {
            pk_propagate(pk_vm_inc(vm));
            PKAtom atom = pk_vm_get_atom(vm, pk_vat(vm));
            pk_propagate(atom);
            pk_propagate(pk_stack_push(atom));
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_LOAD_NIL: {
            pk_propagate(pk_stack_push(pk_nil()));
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_LOAD_TRUE: {
            pk_propagate(pk_stack_push(pk_true()));
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_CALL: {
            pk_u8 args = 0;
            pk_propagate(pk_vm_inc(vm));
            args = pk_vat(vm);
            PKAtom result = pk_stack_funcall(PK_CALL_FUN, args);
            pk_propagate(result);
            pk_propagate(pk_stack_push(result));
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_BLOCK_BEGIN: {
            pk_propagate(pk_frame_push(0));
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_BLOCK_END: {
            PKAtom result = pk_stack_head();
            pk_frame_unwind();
            pk_propagate(pk_stack_push(result));
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_BLOCK_CLEAR: {
            pk_frame_clear();
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_JMP_IF_NIL: {
            pk_propagate(pk_vm_inc(vm));
            PKAtom atom = pk_stack_head();
            pk_propagate(atom);

            if (pk_is_nil(atom)){
                pk_propagate(pk_vm_jmp(vm, pk_vat(vm)));
            } else {
                pk_propagate(pk_vm_inc(vm));
            }
            break;
        }
        case PK_OP_JMP: {
            pk_propagate(pk_vm_inc(vm));
            pk_propagate(pk_vm_jmp(vm, pk_vat(vm)));
            break;
        }
        case PK_OP_JMP_BACK: {
            pk_propagate(pk_vm_inc(vm));
            pk_propagate(pk_vm_jmp_back(vm, pk_vat(vm)));
            break;
        }
        case PK_OP_LET_VAR: {
            pk_propagate(pk_vm_op_let(vm, PK_ENV_VAR));
            break;
        }
        case PK_OP_LET_FUN: {
            pk_propagate(pk_vm_op_let(vm, PK_ENV_FUN));
            break;
        }
        case PK_OP_LET_PROP: {
            pk_propagate(pk_vm_op_let(vm, PK_ENV_PROP));
            break;
        }
        case PK_OP_GET_VAR: {
            pk_propagate(pk_vm_op_get(vm, PK_ENV_VAR));
            break;
        }
        case PK_OP_GET_FUN: {
            pk_propagate(pk_vm_op_get(vm, PK_ENV_FUN));
            break;
        }
        case PK_OP_GET_PROP: {
            pk_propagate(pk_vm_op_get(vm, PK_ENV_PROP));
            break;
        }
        case PK_OP_SET_VAR: {
            pk_propagate(pk_vm_op_set(vm, PK_ENV_VAR));
            break;
        }
        case PK_OP_SET_FUN: {
            pk_propagate(pk_vm_op_set(vm, PK_ENV_FUN));
            break;
        }
        case PK_OP_SET_PROP: {
            pk_propagate(pk_vm_op_set(vm, PK_ENV_PROP));
            break;
        }
        case PK_OP_MAKE_LIST: {
            pk_propagate(pk_vm_inc(vm));
            pk_propagate(pk_stack_op_list(pk_vat(vm)));
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_MAKE_LIST_PACKED: {
            pk_propagate(pk_vm_inc(vm));
            pk_propagate(pk_stack_op_list_packed(pk_vat(vm)));
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_MERGE_LISTS: {
            pk_propagate(pk_vm_inc(vm));
            pk_propagate(pk_stack_op_list_merge(pk_vat(vm)));
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_STRCAT: {
            pk_propagate(pk_vm_inc(vm));
            pk_propagate(pk_stack_op_strcat(pk_vat(vm)));
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_RET: {
            vm->finished = true;
            break;
        }
        default: {
            pk_propagate(pk_error());
            break;
        }
    }
    
    return pk_true();
}

PKAtom pk_vm_exec(PKVM *vm) {
    // The frame starts below this lfun's arguments, so that a leading LET_VAR
    // can see them in the frame's slice.
    pk_propagate(pk_frame_push(vm->save));
    
    while (!vm->finished) {
        PKAtom atom = pk_vm_op_exec(vm);
        if (pk_is_error(atom)) {
            pk_frame_unwind();
            return atom;
        }
    }
    PKAtom result = pk_stack_head();
    pk_frame_unwind();
    return result;
}

PKAtom pk_lfun_exec(PKAtom lfun, size_t save) {
    PKVM vm = pk_vm_init(lfun, save);
    return pk_vm_exec(&vm);
}
