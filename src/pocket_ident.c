#include "pocket_internals.h"

PKStr pk_ident_atomty(pk_u8 type) {
    switch (type) {
        case PK_ATOMTY_ILLEGAL: return pk_str("ILLEGAL");
        case PK_ATOMTY_FREE: return pk_str("FREE");
        case PK_ATOMTY_UNDEFINED: return pk_str("UNDEFINED");
        case PK_ATOMTY_NIL: return pk_str("NIL");
        case PK_ATOMTY_TRUE: return pk_str("TRUE");
        case PK_ATOMTY_NUMBER: return pk_str("NUMBER");
        case PK_ATOMTY_STRING: return pk_str("STRING");
        case PK_ATOMTY_SYMBOL: return pk_str("SYMBOL");
        case PK_ATOMTY_KEYWORD: return pk_str("KEYWORD");
        case PK_ATOMTY_CONS: return pk_str("CONS");
        case PK_ATOMTY_ARRAY: return pk_str("ARRAY");
        case PK_ATOMTY_CFUN: return pk_str("CFUN");
        case PK_ATOMTY_LFUN: return pk_str("LFUN");
        case PK_ATOMTY_CMACRO: return pk_str("CMACRO");
        case PK_ATOMTY_LMACRO: return pk_str("LMACRO");
        case PK_ATOMTY_OBJECT: return pk_str("OBJECT");
        case PK_ATOMTY_BLOB: return pk_str("BLOB");
        case PK_ATOMTY_DESTROYED: return pk_str("DESTROYED");
        default: return pk_str("UNKNOWN");
    }
}

PKStr pk_ident_type(PK_TYPE type) {
    switch (type) {
        case PK_TYPE_UNKNOWN: return pk_str("Unknown");
        case PK_TYPE_DESTROYED: return pk_str("Destroyed");
        case PK_TYPE_NIL: return pk_str("Nil");
        case PK_TYPE_TRUE: return pk_str("True");
        case PK_TYPE_NUMBER: return pk_str("Number");
        case PK_TYPE_STRING: return pk_str("String");
        case PK_TYPE_SYMBOL: return pk_str("Symbol");
        case PK_TYPE_KEYWORD: return pk_str("Keyword");
        case PK_TYPE_CONS: return pk_str("Cons");
        case PK_TYPE_ARRAY: return pk_str("Array");
        case PK_TYPE_OBJECT: return pk_str("Object");
        case PK_TYPE_FUNCTION: return pk_str("Function");
        case PK_TYPE_MACRO: return pk_str("Macro");
        case PK_TYPE_BLOB: return pk_str("Blob");
        default: return pk_str("Unknown");
    }
}

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
        case PK_OP_JMP_UNLESS: return pk_str("JMP_UNLESS");
        case PK_OP_JMP_WHEN: return pk_str("JMP_WHEN");
        case PK_OP_JMP: return pk_str("JMP");
        case PK_OP_JMP_BACK: return pk_str("JMP_BACK");
        case PK_OP_LET: return pk_str("LET");
        case PK_OP_FLET: return pk_str("FLET");
        case PK_OP_FETCH: return pk_str("FETCH");
        case PK_OP_FFETCH: return pk_str("FFETCH");
        case PK_OP_SET_VAR: return pk_str("SET_VAR");
        case PK_OP_SET_FUN: return pk_str("SET_FUN");
        case PK_OP_MAKE_LIST: return pk_str("MAKE_LIST");
        case PK_OP_MAKE_LIST_PACKED: return pk_str("MAKE_LIST_PACKED");
        case PK_OP_MERGE_LISTS: return pk_str("MERGE_LISTS");
        case PK_OP_STRCAT: return pk_str("STRCAT");
        case PK_OP_DIG: return pk_str("DIG");
        case PK_OP_CAR: return pk_str("CAR");
        case PK_OP_CAR_STRICT: return pk_str("CAR_STRICT");
        case PK_OP_CDR: return pk_str("CDR");
        case PK_OP_CDR_STRICT: return pk_str("CDR_STRICT");
        case PK_OP_PROTECT_BEGIN: return pk_str("PROTECT_BEGIN");
        case PK_OP_PROTECT_END: return pk_str("PROTECT_END");
        case PK_OP_BLOCK_POP: return pk_str("BLOCK_POP");
        default: return pk_str("UNKNOWN");
    }
}

PKStr pk_ident_arity(pk_u8 arity) {
    switch (arity) {
        case PK_INT_ARITY_NORMAL: return pk_str("normal");
        case PK_INT_ARITY_OPTIONAL: return pk_str("optional");
        case PK_INT_ARITY_VARIADIC: return pk_str("variadic");
        default: return pk_str("unknown");
    }
}

PKStr pk_ident_env(PK_ENV env) {
    switch (env) {
        case PK_ENV_VAR: return pk_str("VAR");
        case PK_ENV_FUN: return pk_str("FUN");
        default: return pk_str("UNKNOWN");
    }
}

PK_OPCODE_TY pk_opcode_ty(pk_u8 op) {
    switch (op) {
        case PK_OP_LOAD: return PK_OPCODE_TY_LOAD;
        case PK_OP_LOAD_NIL: return PK_OPCODE_TY_NORMAL;
        case PK_OP_LOAD_TRUE: return PK_OPCODE_TY_NORMAL;
        case PK_OP_FETCH: return PK_OPCODE_TY_LOAD;
        case PK_OP_FFETCH: return PK_OPCODE_TY_LOAD;

        case PK_OP_CALL: return PK_OPCODE_TY_LIT;
        case PK_OP_JMP_UNLESS: return PK_OPCODE_TY_LIT;
        case PK_OP_JMP_WHEN: return PK_OPCODE_TY_LIT;
        case PK_OP_JMP: return PK_OPCODE_TY_LIT;
        case PK_OP_JMP_BACK: return PK_OPCODE_TY_LIT;
        case PK_OP_LET: return PK_OPCODE_TY_LIT;
        case PK_OP_FLET: return PK_OPCODE_TY_LIT;
        case PK_OP_MAKE_LIST: return PK_OPCODE_TY_LIT;
        case PK_OP_MAKE_LIST_PACKED: return PK_OPCODE_TY_LIT;
        case PK_OP_MERGE_LISTS: return PK_OPCODE_TY_LIT;
        case PK_OP_STRCAT: return PK_OPCODE_TY_LIT;
        case PK_OP_SET_VAR: return PK_OPCODE_TY_NORMAL;
        case PK_OP_SET_FUN: return PK_OPCODE_TY_NORMAL;
        case PK_OP_DIG: return PK_OPCODE_TY_LIT;
        case PK_OP_POPN: return PK_OPCODE_TY_LIT;
        
        default: return PK_OPCODE_TY_NORMAL;
    }
}
