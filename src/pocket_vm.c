#include "pocket_internals.h"

#define pk_vat(vm_) ((vm_)->op)

PKVM pk_vm_init(PKAtom lfun, size_t save) {
    return (PKVM) {
        .lfun = lfun,
        .save = save,
        .curr = 0,
        .op = lfun->lfun.bc.e[0],
        .opcode = PK_OP_ILLEGAL,
        .finished = false,
        .protected = false,
        .protect_offset = 0,
    };
}

PKAtom pk_vm_inc(PKVM *vm) {
    vm->curr += 1;
    if (vm->curr >= vm->lfun->lfun.bc.length) {
        vm->op = PK_OP_ILLEGAL;
        return pk_errors(pk_str("vm out of bounds"));
    }
    vm->op = vm->lfun->lfun.bc.e[vm->curr];
    return pk_true();
}

PKAtom pk_vm_jmp(PKVM *vm, pk_u8 jmp) {
    vm->curr += jmp;
    if (vm->curr >= vm->lfun->lfun.bc.length) {
        vm->op = PK_OP_ILLEGAL;
        return pk_errors(pk_str("vm out of bounds"));
    }
    vm->op = vm->lfun->lfun.bc.e[vm->curr];
    return pk_true();
}

PKAtom pk_vm_jmp_back(PKVM *vm, pk_u8 jmp) {
    if (jmp > vm->curr) {
        vm->op = PK_OP_ILLEGAL;
        return pk_errors(pk_str("vm out of bounds"));
    }
    
    vm->curr -= jmp;
    vm->op = vm->lfun->lfun.bc.e[vm->curr];
    return pk_true();
}

PKAtom pk_vm_get_atom(PKVM *vm, pk_u8 byte) {
    if (byte >= vm->lfun->lfun.atoms.length) {
        return pk_errors(pk_str("bad atom index"));
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

    for (size_t i = 0; i < (let * 2); ++i) {
        pk_ref_dec(slice.e[i]);
    }
    pk_stack_pop_unchecked(let * 2);
    
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
    pk_propagate(pk_set_discard(env, symbol, value));

    PKAtoms popped = PK_ATOMS_EMPTY;
    pk_propagate(pk_stack_slice_down(2, &popped));
    pk_stack_pop_unchecked(1);
    PK_CORE.stack.e[PK_CORE.stack.count - 1] = value;
    pk_propagate(pk_vm_inc(vm));

    return pk_true();
}

PKAtom pk_vm_op_exec(PKVM *vm) {
    vm->opcode = vm->op;
    switch (vm->op) {
        case PK_OP_ILLEGAL: {
            return pk_errors(pk_str("illegal op"));
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
            pk_ref_dec(result);
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
            pk_propagate(pk_ref_inc(result));
            pk_frame_unwind();
            pk_propagate(pk_stack_push(result));
            pk_ref_dec(result);
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_BLOCK_CLEAR: {
            pk_frame_clear();
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_BLOCK_POP: {
            pk_frame_unwind();
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_JMP_UNLESS: {
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
        case PK_OP_JMP_WHEN: {
            pk_propagate(pk_vm_inc(vm));
            PKAtom atom = pk_stack_head();
            pk_propagate(atom);

            if (!pk_is_nil(atom)) {
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
        case PK_OP_DIG: {
            pk_propagate(pk_vm_inc(vm));
            pk_propagate(pk_stack_op_dig(pk_vat(vm)));
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_CAR: {
            pk_propagate(pk_vm_inc(vm));
            pk_propagate(pk_stack_op_car(false));
            break;
        }
        case PK_OP_CAR_STRICT: {
            pk_propagate(pk_vm_inc(vm));
            pk_propagate(pk_stack_op_car(true));
            break;
        }
        case PK_OP_CDR: {
            pk_propagate(pk_vm_inc(vm));
            pk_propagate(pk_stack_op_cdr(false));
            break;
        }
        case PK_OP_CDR_STRICT: {
            pk_propagate(pk_vm_inc(vm));
            pk_propagate(pk_stack_op_cdr(true));
            break;
        }
        case PK_OP_PROTECT_BEGIN: {
            vm->protected = true;
            vm->protect_offset = PK_CORE.stack.count;
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_PROTECT_END: {
            vm->protected = false;
            pk_propagate(pk_vm_inc(vm));
            break;
        }
        case PK_OP_RET: {
            vm->finished = true;
            break;
        }
        default: {
            pk_propagate(pk_errors(pk_str("illegal op")));
            break;
        }
    }
    
    return pk_true();
}

PKAtom pk_vm_exec(PKVM *vm) {
    // The frame starts below this lfun's arguments, so that a leading LET_VAR
    // can see them in the frame's slice.
    size_t frames = PK_CORE.frames.count;
    pk_propagate(pk_frame_push(vm->save));
    
    while (!vm->finished) {
        PKAtom atom = pk_vm_op_exec(vm);
        if (pk_is_throwing(atom)) {
            if ((!vm->protected) || pk_is_critical_error(atom)) {
                while (PK_CORE.frames.count > frames) {
                    pk_frame_unwind();
                }
                return atom;
            }
            
            // The protected region handles the throw itself, so the value the
            // op was working on is dropped and the next form carries on.
            pk_ref_dec(atom);
            pk_stack_reset_to(vm->protect_offset);
            pk_propagate(pk_stack_push(pk_nil()));
            if (pk_opcode_ty(vm->opcode) == PK_OPCODE_TY_LIT) {
                pk_propagate(pk_vm_inc(vm));
            }
            continue;
        }
    }
    // The frame is about to be released and it owns the result, so hand that
    // reference to the caller.
    PKAtom result = pk_stack_head();
    pk_propagate(pk_ref_inc(result));
    pk_frame_unwind();
    return result;
}

