#include "config.h"
#include "input.h"
#include "log.h"

#include "input-method-unstable-v2-client-protocol.h"
#include <wayland-client.h>

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

const char *usage_msg =
    "usage: wlabbr [options]\n"
    "\t-c, --config <file>  specify json config file\n"
    "\t-h, --help\t     show this help message\n"
    "\t-v, --verbose\t     verbose logging\n";

struct {
  struct wl_seat *seat;
  struct zwp_input_method_manager_v2 *input_method_manager;
} static objects;

void registry_handle_global(
    void *data, struct wl_registry *registry, uint32_t name,
    const char *interface, uint32_t version
) {
  if (strcmp(interface, zwp_input_method_manager_v2_interface.name) == 0) {
    objects.input_method_manager = wl_registry_bind(
        registry, name, &zwp_input_method_manager_v2_interface, version
    );
  }
  if (strcmp(interface, wl_seat_interface.name) == 0) {
    objects.seat =
        wl_registry_bind(registry, name, &wl_seat_interface, version);
  }
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = NULL,
};

int main(int argc, char *argv[]) {
  // disabling stderr logging in getopt on invalid input
  opterr = 0;
  struct option const long_opts[] = {
      {"config", required_argument, NULL, 'c'},
      {"help", no_argument, NULL, 'h'},
      {"verbose", no_argument, NULL, 'v'},
      {0},
  };

  char const *config_addr = NULL;

  int opt = 0;
  while ((opt = getopt_long(argc, argv, "c:hv", long_opts, NULL)) != -1) {
    switch (opt) {
    case 'c':
      config_addr = optarg;
      break;
    // ':'-missing opt, '?'-unknown opt
    case ':':
    case '?':
    case 'h':
      printf("%s", usage_msg);
      return EXIT_SUCCESS;
    case 'v':
      log_init(INFO);
      break;
    }
  }
  log_report(INFO, "filename: %s", config_addr);

  config_init(config_addr);

  struct wl_display *const display = wl_display_connect(NULL);

  if (!display) {
    log_report(ERROR, "could not connect to wl_display");
    return EXIT_FAILURE;
  }

  struct wl_registry *const registry = wl_display_get_registry(display);
  if (!registry) {
    log_report(ERROR, "could not wl_registry");
    return EXIT_FAILURE;
  }

  wl_registry_add_listener(registry, &registry_listener, NULL);
  wl_display_roundtrip(display);

  if (!objects.seat || !objects.input_method_manager) {
    log_report(
        ERROR, "missing wayland interfaces, ensure display server compatibility"
    );
    return EXIT_FAILURE;
  }

  bool *const running = input_init(objects.input_method_manager, objects.seat);

  while (*running && wl_display_dispatch(display) != -1) {
    ;
  }

  config_destroy();

  zwp_input_method_manager_v2_destroy(objects.input_method_manager);
  wl_registry_destroy(registry);
  wl_display_disconnect(display);

  return EXIT_SUCCESS;
}
