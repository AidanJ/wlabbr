#pragma once
#include <stdbool.h>

#include "input-method-unstable-v2-client-protocol.h"

// returned flag's memory should not be managed
bool* input_init(
    struct zwp_input_method_manager_v2 *input_method_manager,
    struct wl_seat *seat
);
