#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>

#include <xdp/libxdp.h>

#include "config.h"
#include "device.h"
#include "xdp_prog_user.h"

struct xdp_program *load_bpf_and_xdp_attach() {
  /* In next assignment this will be moved into ../common/ */
  int prog_fd = -1;
  int err;
  struct bpf_object *obj = NULL;

  const struct config *conf = global_config_get();
  int ifindex = xdp_device_index_get();

  // Clear previous prog
  struct xdp_multiprog *mp = xdp_multiprog__get_from_ifindex(ifindex);
  err = libxdp_get_error(mp);
  if (!err) {
    err = xdp_multiprog__detach(mp);
    if (err != 0) {
      perror("xdp_multiprog__detach failed.");
      exit(EXIT_FAILURE);
    }
  }

  obj = bpf_object__open(conf->open_filename);
  if (obj == NULL) {
    perror("bpf_object__open failed");
    exit(EXIT_FAILURE);
  }

  struct xdp_program_opts prog_opts = {};
  prog_opts.sz = sizeof(struct xdp_program_opts);
  prog_opts.obj = obj;
  prog_opts.prog_name = conf->prog_name;

  struct xdp_program *prog = xdp_program__create(&prog_opts);
  if (prog == NULL) {
    perror("xdp_program__create failed");
    exit(EXIT_FAILURE);
  }

  err = xdp_program__attach(prog, ifindex, XDP_MODE_UNSPEC, 0);
  if (err != 0) {
    perror("xdp_program__attach failed");
    exit(EXIT_FAILURE);
  }

  prog_fd = xdp_program__fd(prog);
  if (prog_fd < 0) {
    fprintf(stderr, "ERR: xdp_program__fd failed: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  return prog;
}

void redirect_rule_set(struct xdp_program *prog) {
  int ret = 0;
  const struct config *conf = global_config_get();
  const struct device *device_inst = device_inst_get();
  struct bpf_object *obj = xdp_program__bpf_obj(prog);

  struct bpf_map *redirect_map = bpf_object__find_map_by_name(obj, "dummy_redirect_map");
  if (redirect_map == NULL) {
    perror("bpf_object__find_map_by_name look for redirect_map failed");
    goto failure;
  }
  int redirect_map_fd = bpf_map__fd(redirect_map);

  for (unsigned int i = 0; i < device_inst->nr_device; i++) {
    const struct device_desc *device_ref = &device_inst->device_descs[i];
    const int device_index = device_inst->device_index[i];

    if (device_ref->key > 0) {
      ret = bpf_map_update_elem(redirect_map_fd, &device_ref->key, &device_index, 0);
      if (ret != 0) {
        printf("fail to insert redirect rule");
        goto failure;
      }
    }
  }

  return;

failure:
  exit(EXIT_FAILURE);
}