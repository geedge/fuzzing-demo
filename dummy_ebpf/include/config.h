#pragma once
#include <linux/limits.h>

#include "device.h"

#define PROG_NAME_MAXSIZE 32

struct config {
  int nr_device;
  struct device_desc devices[DEVICE_MAX_NUM];
  char open_filename[PATH_MAX];
  char prog_name[PROG_NAME_MAXSIZE];
};

void global_config_init();
const struct config *global_config_get();