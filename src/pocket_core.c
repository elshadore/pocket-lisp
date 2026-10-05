#include "pocket_internals.h"

PKCore PK_CORE = (PKCore) {
    .pool = NULL,
    .free = NULL,
    .initialized = false,
};

bool pk_init() {
    if (PK_CORE.initialized) {
        return true;
    }
    PK_CORE.initialized = true;
    return true;
}

void pk_deinit() {
    if (!PK_CORE.initialized) {
        return;
    }
}
