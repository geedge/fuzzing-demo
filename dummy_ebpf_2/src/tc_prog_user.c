#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>

#include <xdp/libxdp.h>

#include "config.h"
#include "device.h"
#include "tc_prog_user.h"

static bool created_hook = true;
struct bpf_tc_hook hook = {};
struct bpf_tc_opts tc_opt = {};

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args) {
  return vfprintf(stderr, format, args);
}

struct bpf_object *load_bpf_and_tc_attach() {
  int err;

  const struct config *conf = global_config_get();
  int ifindex = xdp_device_index_get();

  hook.sz = sizeof(struct bpf_tc_hook);
  hook.attach_point = BPF_TC_EGRESS;
  tc_opt.sz = sizeof(struct bpf_tc_opts);

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

  //
  // export LIBBPF_LOG_LEVEL=info
  // bpf_program__set_log_level(prog, LIBBPF_DEBUG);
  // libbpf_set_print(libbpf_print_fn);

  tc_opt.prog_fd = prog_fd;

  hook.ifindex = ifindex;

  err = bpf_tc_hook_create(&hook);
  if (err && err == -EEXIST) {
    created_hook = false;
  } else if (err) {
    fprintf(stderr, "Couldn't create hook for ifindex %d\n", ifindex);
    exit(EXIT_FAILURE);
  }

  err = bpf_tc_attach(&hook, &tc_opt);
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

void tc_dettach(struct bpf_object *obj) {
  // ref: bpf-examples/tc-policy/tc_txq_policy.c

  int err = 0;
  int ifindex = xdp_device_index_get();
  const struct config *conf = global_config_get();

  // look at the source code of libbpf, it must be cleared to zero. otherwise it is considered
  // an illegal parameter.
  tc_opt.prog_fd = 0;
  tc_opt.prog_id = 0;
  tc_opt.flags = 0;

  // opt's hanlde is important
  err = bpf_tc_query(&hook, &tc_opt);
  if (err == 0) {
    printf("BPF program found. its program id is: %d\n", tc_opt.prog_id);
  } else {
    printf("no BPF program need to dettach: %d\n", err);
    return;
  }

  tc_opt.prog_id = 0;
  err = bpf_tc_detach(&hook, &tc_opt);
  if (err == 0) {
    printf("detach %s successfully.\n", conf->prog_name);
  } else {
    printf("detach %s failed:%s\n", conf->prog_name, strerror(errno));
  }

  if (created_hook) {
    // Even if this function executes successfully, qdisc clsact still exists
    err = bpf_tc_hook_destroy(&hook);
    if (err) {
      fprintf(stderr, "bpf_tc_hook_destroy failed %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  bpf_object__close(obj);
}