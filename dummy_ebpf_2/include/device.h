#pragma once
#include <net/if.h>
#include <stdint.h>

#define DEVICE_MAX_NUM 10

struct device_desc {
  int key;
  char device_type[20];
  char device_name[IF_NAMESIZE];
};

struct device {
  int nr_device;
  int device_index[DEVICE_MAX_NUM];
  struct device_desc device_descs[DEVICE_MAX_NUM];

  int device_fds[DEVICE_MAX_NUM];
};

void device_init();
const struct device *device_inst_get();
int xdp_device_index_get();
struct device_desc *device_desc_find_by_fd(int fd);