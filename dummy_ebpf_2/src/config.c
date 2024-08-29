#include <stdio.h>
#include <string.h>

#include "config.h"

static struct config global_conf = {};

void global_config_init() {
  // There are three devices in the configuration by default
  global_conf.nr_device = 3;

  struct device_desc *device_ref = &global_conf.devices[0];
  device_ref->key = 0;
  snprintf(device_ref->device_type, sizeof(device_ref->device_type), "%s", "dummy");
  snprintf(device_ref->device_name, sizeof(device_ref->device_name), "%s", "dummy0");

  device_ref = &global_conf.devices[1];
  device_ref->key = '0';
  snprintf(device_ref->device_type, sizeof(device_ref->device_type), "%s", "tap");
  snprintf(device_ref->device_name, sizeof(device_ref->device_name), "%s", "tap0");

  device_ref = &global_conf.devices[2];
  device_ref->key = '1';
  snprintf(device_ref->device_type, sizeof(device_ref->device_type), "%s", "tap");
  snprintf(device_ref->device_name, sizeof(device_ref->device_name), "%s", "tap1");

  // The file where ebpf is located, and the ebpf program name
  snprintf(global_conf.open_filename, sizeof(global_conf.open_filename), "%s", "tc_prog_kernel.o");
  snprintf(global_conf.prog_name, sizeof(global_conf.prog_name), "%s", "tc_redirect_map_func");
}

const struct config *global_config_get() { return &global_conf; }