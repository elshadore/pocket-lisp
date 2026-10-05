#include "ulisp_internals.h"

ULCore UL_CORE = (ULCore) {
    .pool = NULL,
    .free = NULL,
    .initialized = false,
};

bool ul_init() {
    if (UL_CORE.initialized) {
        return true;
    }
    UL_CORE.initialized = true;
    return true;
}

void ul_deinit() {
    if (!UL_CORE.initialized) {
        return;
    }
}
