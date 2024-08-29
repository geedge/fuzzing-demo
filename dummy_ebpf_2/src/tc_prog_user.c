#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>

#include <xdp/libxdp.h>

#include "config.h"
#include "device.h"
#include "tc_prog_user.h"

struct bpf_object *load_bpf_and_tc_attach() {
  int err;

  const struct config *conf = global_config_get();
  int ifindex = xdp_device_index_get();

  DECLARE_LIBBPF_OPTS(bpf_tc_hook, hook, .attach_point = BPF_TC_EGRESS);
  DECLARE_LIBBPF_OPTS(bpf_tc_opts, attach);

  // Clear previous prog
  tc_dettach();

  struct bpf_object *obj = bpf_object__open(conf->open_filename);
  if (obj == NULL) {
    perror("bpf_object__open failed");
    exit(EXIT_FAILURE);
  }

  err = bpf_object__load(obj);
  if (err) {
    fprintf(stderr, "Failed to load object\n");
    exit(EXIT_FAILURE);
  }

  struct bpf_program *prog = bpf_object__find_program_by_name(obj, "tc_redirect_map_func");
  int prog_fd = bpf_program__fd(prog);
  if (prog_fd < 0) {
    fprintf(stderr, "Couldn't find program 'tc_redirect_map_func'\n");
    exit(EXIT_FAILURE);
  }

  attach.prog_fd = prog_fd;

  hook.ifindex = ifindex;

  err = bpf_tc_hook_create(&hook);
  if (err && err != -EEXIST) {
    fprintf(stderr, "Couldn't create hook for ifindex %d\n", ifindex);
    exit(EXIT_FAILURE);
  }

  err = bpf_tc_attach(&hook, &attach);
  if (err) {
    fprintf(stderr, "Couldn't attach program to ifindex %d\n", hook.ifindex);
    exit(EXIT_FAILURE);
  }

  return obj;
}

void redirect_rule_set(struct bpf_object *obj) {
  int ret = 0;
  const struct config *conf = global_config_get();
  const struct device *device_inst = device_inst_get();

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

void tc_dettach() {
  int err = 0;
  int ifindex = xdp_device_index_get();

  DECLARE_LIBBPF_OPTS(bpf_tc_hook, hook, .attach_point = BPF_TC_EGRESS);
  DECLARE_LIBBPF_OPTS(bpf_tc_opts, opts);

  hook.ifindex = ifindex;
  opts.prog_fd = -1;

  err = bpf_tc_query(&hook, NULL);
  if (err == 0) {
    printf("BPF program found with FD: %d\n", opts.prog_fd);
  } else {
    printf("no BPF program need to dettach: %d\n", err);
    return;
  }

  err = bpf_tc_hook_destroy(&hook);
  if (err && err != -EEXIST) {
    fprintf(stderr, "bpf_tc_hook_destroy failed %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
}