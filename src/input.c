#include "input.h"
#include "config.h"
#include "log.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <time.h>

#include "input-method-unstable-v2-client-protocol.h"
#include <wayland-client.h>

// TODO add `input_state_init`
static struct {
  struct wl_seat *seat;
  struct zwp_input_method_manager_v2 *input_method_manager;
  struct zwp_input_method_v2 *input_method;

  bool running;

  struct {
    char *text;
    uint32_t cursor;
    uint32_t anchor;
  } surrounding;

  struct {
    uint32_t cursor;
  } last_expand;

  bool active;
  uint32_t serial;
} state;

// NOTE static struct is already 0 initialized by default so some of the
// explicit initializations are unnecessary
static void input_init_state(
    struct wl_seat *seat,
    struct zwp_input_method_manager_v2 *input_method_manager
) {
  state.seat = seat;
  state.input_method_manager = input_method_manager;
  state.input_method = zwp_input_method_manager_v2_get_input_method(
      state.input_method_manager, state.seat
  );

  state.running = true;
  state.last_expand.cursor = 0;

  state.surrounding.text = NULL;
  state.active = false;
  state.serial = 0;
}

static void input_state_destroy() {
  state.running = false;
  zwp_input_method_v2_destroy(state.input_method);
}

// searches for the last space occurrence (or beginning of surrounding text)
// from the left of the cursor position
static uint32_t input_get_left_bound() {
  if (state.surrounding.cursor > 1) {
    for (long idx = state.surrounding.cursor - 2; idx >= 0; --idx) {
      if (state.surrounding.text[idx] == ' ') {
        return idx + 1;
      }
    }
  }
  return 0;
}

// searches for the next space occurrence (or ending of surrounding text) from
// the left of the cursor position
static uint32_t input_get_right_bound() {
  // printf("rb End: %u\n", state.surrounding.cursor - 1);
  if (state.surrounding.cursor != strlen(state.surrounding.text)) {
    for (uint32_t idx = state.surrounding.cursor;
         idx < strlen(state.surrounding.text); ++idx) {
      if (state.surrounding.text[idx] == ' ') {
        return idx - 1;
      }
    }
  }
  return strlen(state.surrounding.text) - 1;
}

static void
input_handle_activate(void *data, struct zwp_input_method_v2 *input_method) {
  state.active = true;
}

static void
input_handle_deactivate(void *data, struct zwp_input_method_v2 *input_method) {
  state.active = false;
}

// TODO Define max string size (<= 4'000)
static void input_handle_surrounding_text(
    void *data, struct zwp_input_method_v2 *input_method, const char *string,
    uint32_t cursor, uint32_t anchor
) {
  free(state.surrounding.text);
  state.surrounding.text = malloc(strlen(string) + 1);
  // using memcpy to also copy null terminator
  memcpy(state.surrounding.text, string, strlen(string) + 1);
  state.surrounding.cursor = cursor;
  state.surrounding.anchor = anchor;
}

static void input_handle_text_change_cause(
    void *data, struct zwp_input_method_v2 *input_method, uint32_t cause
) {}

static void input_handle_content_type(
    void *data, struct zwp_input_method_v2 *input_method, uint32_t /*unused*/,
    uint32_t
    /*unused*/
) {}

// TODO Destroy on sigkill
// NOTE: scanning is currently built for live typing - no completions for pasted
// text
static void
input_handle_done(void *data, struct zwp_input_method_v2 *input_method) {
  ++state.serial;

  // cant commit string
  if (!state.active) {
    return;
  }
  // no text avaliable for scanning
  if (state.surrounding.cursor == 0) {
    return;
  }

  uint32_t const left_bound = input_get_left_bound();
  uint32_t const right_bound = input_get_right_bound();
  char const *const expanded =
      config_keywords_match(state.surrounding.text, left_bound, right_bound);

  printf(
      "last: %u, now: %u\n", state.last_expand.cursor, state.surrounding.cursor
  );

  // invalid keyword
  if (!expanded) {
    return;
  }

  // avoiding recursive completion
  if (state.last_expand.cursor &&
      state.last_expand.cursor == state.surrounding.cursor) {
    log_report(
        WARNING, "cancelled completion (%s) due to recursive expansion",
        expanded
    );
    state.last_expand.cursor = 0;
    return;
  }

  state.last_expand.cursor = state.surrounding.cursor + strlen(expanded) -
                             (right_bound - left_bound + 1);

  // TODO make sure not modified while commiting?
  zwp_input_method_v2_delete_surrounding_text(
      state.input_method, state.surrounding.cursor - left_bound,
      right_bound - (state.surrounding.cursor - 1)
  );
  zwp_input_method_v2_commit_string(state.input_method, expanded);
  zwp_input_method_v2_commit(state.input_method, state.serial);
}

static void
input_handle_unavailable(void *data, struct zwp_input_method_v2 *input_method) {
  input_state_destroy();
}

static const struct zwp_input_method_v2_listener input_method_listener = {
    .activate = input_handle_activate,
    .deactivate = input_handle_deactivate,
    .surrounding_text = input_handle_surrounding_text,
    .text_change_cause = input_handle_text_change_cause,
    .content_type = input_handle_content_type,
    .done = input_handle_done,
    .unavailable = input_handle_unavailable
};

bool *input_init(
    struct zwp_input_method_manager_v2 *input_method_manager,
    struct wl_seat *seat
) {
  input_init_state(seat, input_method_manager);
  config_keywords_init();

  zwp_input_method_v2_add_listener(
      state.input_method, &input_method_listener, NULL
  );

  return &state.running;
}
