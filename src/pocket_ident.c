#include "pocket_internals.h"

PKStr pk_ident_opcode(pk_u8 op) {
    switch (op) {
        case PK_OP_ILLEGAL: return pk_str("ILLEGAL");
        case PK_OP_NOP: return pk_str("NOP");
        case PK_OP_RET: return pk_str("RET");
        case PK_OP_LOAD: return pk_str("LOAD");
        case PK_OP_LOAD_NIL: return pk_str("LOAD_NIL");
        case PK_OP_LOAD_TRUE: return pk_str("LOAD_TRUE");
        case PK_OP_CALL: return pk_str("CALL");
        case PK_OP_BLOCK_BEGIN: return pk_str("BLOCK_BEGIN");
        case PK_OP_BLOCK_END: return pk_str("BLOCK_END");
        case PK_OP_BLOCK_CLEAR: return pk_str("BLOCK_CLEAR");
        case PK_OP_JMP_IF_NIL: return pk_str("JMP_IF_NIL");
        case PK_OP_JMP: return pk_str("JMP");
        case PK_OP_JMP_BACK: return pk_str("JMP_BACK");
        case PK_OP_LET_VAR: return pk_str("LET_VAR");
        case PK_OP_LET_FUN: return pk_str("LET_FUN");
        case PK_OP_LET_PROP: return pk_str("LET_PROP");
        case PK_OP_GET_VAR: return pk_str("GET_VAR");
        case PK_OP_GET_FUN: return pk_str("GET_FUN");
        case PK_OP_GET_PROP: return pk_str("GET_PROP");
        case PK_OP_SET_VAR: return pk_str("SET_VAR");
        case PK_OP_SET_FUN: return pk_str("SET_FUN");
        case PK_OP_SET_PROP: return pk_str("SET_PROP");
        case PK_OP_MAKE_LIST: return pk_str("MAKE_LIST");
        case PK_OP_MAKE_LIST_PACKED: return pk_str("MAKE_LIST_PACKED");
        case PK_OP_MERGE_LISTS: return pk_str("MERGE_LISTS");
        case PK_OP_STRCAT: return pk_str("STRCAT");
        default: return pk_str("UNKNOWN");
    }
}

PKStr pk_ident_atomty(pk_u8 ty) {
    switch (ty) {
        case PK_ATOMTY_ILLEGAL: return pk_str("ILLEGAL");
        case PK_ATOMTY_TYPE: return pk_str("TYPE");
        case PK_ATOMTY_FREE: return pk_str("FREE");
        case PK_ATOMTY_THROW: return pk_str("THROW");
        case PK_ATOMTY_UNDEFINED: return pk_str("UNDEFINED");
        case PK_ATOMTY_NIL: return pk_str("NIL");
        case PK_ATOMTY_TRUE: return pk_str("TRUE");
        case PK_ATOMTY_NUMBER: return pk_str("NUMBER");
        case PK_ATOMTY_STRING: return pk_str("STRING");
        case PK_ATOMTY_SYMBOL: return pk_str("SYMBOL");
        case PK_ATOMTY_KEYWORD: return pk_str("KEYWORD");
        case PK_ATOMTY_CONS: return pk_str("CONS");
        case PK_ATOMTY_ARRAY: return pk_str("ARRAY");
        case PK_ATOMTY_TABLE: return pk_str("TABLE");
        case PK_ATOMTY_CFUN: return pk_str("CFUN");
        case PK_ATOMTY_LFUN: return pk_str("LFUN");
        default: return pk_str("UNKNOWN");
    }
}

PKStr pk_ident_arity(pk_u8 arity) {
    switch (arity) {
        case PK_ARITY_NORMAL: return pk_str("NORMAL");
        case PK_ARITY_OPTIONAL: return pk_str("OPTIONAL");
        case PK_ARITY_VARIADIC: return pk_str("VARIADIC");
        default: return pk_str("UNKNOWN");
    }
}

PKStr pk_ident_env(PK_ENV env) {
    switch (env) {
        case PK_ENV_VAR: return pk_str("VAR");
        case PK_ENV_FUN: return pk_str("FUN");
        case PK_ENV_PROP: return pk_str("PROP");
        default: return pk_str("UNKNOWN");
    }
}

PK_OPCODE_TY pk_opcode_ty(pk_u8 op) {
    switch (op) {
        case PK_OP_LOAD: return PK_OPCODE_TY_LOAD;
        case PK_OP_LOAD_NIL: return PK_OPCODE_TY_LOAD;
        case PK_OP_LOAD_TRUE: return PK_OPCODE_TY_LOAD;
        case PK_OP_GET_VAR: return PK_OPCODE_TY_LOAD;
        case PK_OP_GET_FUN: return PK_OPCODE_TY_LOAD;
        case PK_OP_GET_PROP: return PK_OPCODE_TY_LOAD;
        
        case PK_OP_CALL: return PK_OPCODE_TY_LIT;
        case PK_OP_JMP_IF_NIL: return PK_OPCODE_TY_LIT;
        case PK_OP_JMP: return PK_OPCODE_TY_LIT;
        case PK_OP_JMP_BACK: return PK_OPCODE_TY_LIT;
        case PK_OP_LET_VAR: return PK_OPCODE_TY_LIT;
        case PK_OP_LET_FUN: return PK_OPCODE_TY_LIT;
        case PK_OP_MAKE_LIST: return PK_OPCODE_TY_LIT;
        case PK_OP_MAKE_LIST_PACKED: return PK_OPCODE_TY_LIT;
        case PK_OP_MERGE_LISTS: return PK_OPCODE_TY_LIT;
        case PK_OP_STRCAT: return PK_OPCODE_TY_LIT;
        case PK_OP_LET_PROP: return PK_OPCODE_TY_LIT;
        case PK_OP_SET_VAR: return PK_OPCODE_TY_LIT;
        case PK_OP_SET_FUN: return PK_OPCODE_TY_LIT;
        case PK_OP_SET_PROP: return PK_OPCODE_TY_LIT;
        
        default: return PK_OPCODE_TY_NORMAL;
    }
}
