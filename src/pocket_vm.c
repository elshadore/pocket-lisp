#include "pocket_internals.h"

#define pk_vat(vm_) ((vm_)->op)

#define pk_vm_error(vm_) pk_error((vm_)->lisp)

PKVM pk_vm_init(Pocket lisp, PKAtomLFunc *lfunc) {
    PKVM vm;
    vm.lisp = lisp;
    vm.lfunc = lfunc;
    vm.curr = 0;
    vm.op = lfunc->bc.e[0];
    return vm;
}

PKRes pk_vm_inc(PKVM *vm) {
    vm->curr += 1;
    if (vm->curr >= vm->lfunc->bc.length) {
        vm->op = PK_OP_ILLEGAL;
        return pk_vm_error(vm);
    }
    vm->op = vm->lfunc->bc.e[vm->curr];
    return PK_Ok;
}

PKRes pk_vm_jmp(PKVM *vm, pk_u8 jmp) {
    vm->curr += jmp;
    if (vm->curr >= vm->lfunc->bc.length) {
        vm->op = PK_OP_ILLEGAL;
        return pk_vm_error(vm);
    }
    vm->op = vm->lfunc->bc.e[vm->curr];
    return PK_Ok;
}

PKRes pk_vm_jmp_back(PKVM *vm, pk_u8 jmp) {
    if (jmp > vm->curr) {
        vm->op = PK_OP_ILLEGAL;
        return pk_vm_error(vm);
    }
    
    vm->curr -= jmp;
    vm->op = vm->lfunc->bc.e[vm->curr];
    return PK_Ok;
}

PKRes pk_vm_get_atom(PKVM *vm, pk_u8 byte, PKAtom **output) {
    if (byte >= vm->lfunc->atoms.length) {
        return pk_vm_error(vm);
    }
    *output = vm->lfunc->atoms.e[byte];
    return PK_Ok;
}

PKRes pk_vm_exec(PKVM *vm) {
    for (;;) {
        switch (pk_vat(vm)) {
            case PK_OP_ILLEGAL: {
                return pk_vm_error(vm);
            }
            case PK_OP_NOP: {
                pk_try(pk_vm_inc(vm));
                break;
            }
            case PK_OP_LOAD: {
                PKAtom *atom = NULL;
                pk_try(pk_vm_inc(vm));
                pk_try(pk_vm_get_atom(vm, pk_vat(vm), &atom));
                pk_try(pk_push(vm->lisp, atom));
                pk_try(pk_vm_inc(vm));
                break;
            }
            case PK_OP_CALL: {
                pk_u8 args = 0;
                pk_try(pk_vm_inc(vm));
                args = pk_vat(vm);
                pk_try(pk_funcall(vm->lisp, args));
                pk_try(pk_vm_inc(vm));
                break;
            }
            case PK_OP_BLOCK_BEGIN: {
                pk_try(pk_frame_push(vm->lisp, 0));
                pk_try(pk_vm_inc(vm));
                break;
            }
            case PK_OP_BLOCK_END: {
                pk_try(pk_return_push(vm->lisp));
                pk_try(pk_vm_inc(vm));
                break;
            }
            case PK_OP_JMP_IF_NIL: {
                PKAtom *atom = NULL;
                
                pk_try(pk_vm_inc(vm));
                pk_try(pk_stack_head(vm->lisp, &atom));
                
                if (pk_atom_is_nil(atom)){
                    pk_try(pk_vm_jmp(vm, pk_vat(vm)));
                } else {
                    pk_try(pk_vm_inc(vm));
                }
                break;
            }
            case PK_OP_JMP: {
                pk_try(pk_vm_inc(vm));
                pk_try(pk_vm_jmp(vm, pk_vat(vm)));
                break;
            }
            case PK_OP_JMP_BACK: {
                pk_try(pk_vm_inc(vm));
                pk_try(pk_vm_jmp_back(vm, pk_vat(vm)));
                break;
            }
            case PK_OP_LET: {
                PKAtomSlice slice;
                size_t let = 0;
                size_t i = 0;
               
                pk_try(pk_vm_inc(vm));
                let = (size_t)pk_vat(vm);
                pk_try(pk_stack_slice_down(vm->lisp, let * 2, &slice));

                for (i = 0; i < let; ++i) {
                    PKAtomSymbol *symbol = NULL;
                    pk_try(pk_atom_cast_symbol(vm->lisp, slice.e[i + let], &symbol));
                    pk_try(pk_let_push(vm->lisp, PKEnvTy_Var, symbol, slice.e[i]));
                }
                
                pk_try(pk_vm_inc(vm));
                
                break;
            }
            case PK_OP_LOOKUP_VAR: {
                PKAtom *atom = NULL;
                PKAtom *result = NULL;
                PKAtomSymbol *symbol = NULL;
                
                pk_try(pk_vm_inc(vm));
                pk_try(pk_vm_get_atom(vm, pk_vat(vm), &atom));
                pk_try(pk_atom_cast_symbol(vm->lisp, atom, &symbol));
                pk_try(pk_env_get(vm->lisp, PKEnvTy_Var, symbol, &result));
                
                pk_try(pk_push(vm->lisp, result));
                pk_try(pk_vm_inc(vm));
                break;
            }
            case PK_OP_LOOKUP_FUN: {
                PKAtom *atom = NULL;
                PKAtom *result = NULL;
                PKAtomSymbol *symbol = NULL;
                
                pk_try(pk_vm_inc(vm));
                pk_try(pk_vm_get_atom(vm, pk_vat(vm), &atom));
                pk_try(pk_atom_cast_symbol(vm->lisp, atom, &symbol));
                pk_try(pk_env_get(vm->lisp, PKEnvTy_Fun, symbol, &result));
                
                pk_try(pk_push(vm->lisp, result));
                pk_try(pk_vm_inc(vm));
                break;
            }
            case PK_OP_RET: {
                return PK_Ok;
            }
            default: {
                return pk_vm_error(vm);
            }
        }
    }
}

PKRes pk_lfunc_exec(Pocket lisp, PKAtomLFunc *lfunc) {
    PKVM vm = pk_vm_init(lisp, lfunc);
    return pk_vm_exec(&vm);
}

