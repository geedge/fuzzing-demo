#include "device.h"
#include "config.h"
#include "tap.h"

#include <assert.h>
#include <linux/if.h>
#include <linux/rtnetlink.h>
#include <netlink/errno.h>
#include <netlink/msg.h>
#include <netlink/netlink.h>
#include <netlink/route/link.h>
#include <netlink/socket.h>

struct device device_inst;

static int netlink_add(const char *iftype, const char *ifname) {
  struct rtnl_link *link = rtnl_link_alloc();
  rtnl_link_set_type(link, iftype);
  rtnl_link_set_name(link, ifname);

  struct nl_sock *sk = nl_socket_alloc();
  nl_connect(sk, NETLINK_ROUTE);

  return rtnl_link_add(sk, link, NLM_F_CREATE | NLM_F_EXCL);
}

static int netlink_change(const char *ifname, unsigned int flags) {
  // modify from: https://github.com/thom311/libnl/blob/main/tests/test-loopback-up-down.c
  struct nl_sock *sk;
  struct rtnl_link *link, *change;
  struct nl_cache *cache;
  int err = 0;

  sk = nl_socket_alloc();
  if ((err = nl_connect(sk, NETLINK_ROUTE)) < 0) {
    nl_perror(err, "Unable to connect socket");
    return err;
  }

  if ((err = rtnl_link_alloc_cache(sk, AF_UNSPEC, &cache)) < 0) {
    nl_perror(err, "Unable to allocate cache");
    goto out;
  }

  if (!(link = rtnl_link_get_by_name(cache, ifname))) {
    fprintf(stderr, "Interface not found\n");
    err = 1;
    goto out;
  }

  /* exit if the loopback interface is already activated */
  /* After calling tap_alloc(), the tap state is already UP */
  err = rtnl_link_get_flags(link);
  if ((err & IFF_UP)) {
    err = 0;
    goto out;
  }

  change = rtnl_link_alloc();
  rtnl_link_set_flags(change, flags);
  if ((err = rtnl_link_change(sk, link, change, 0)) < 0) {
    nl_perror(err, "Unable to activate lo");
    goto out;
  }

  err = 0;

out:
  nl_socket_free(sk);
  return err;
}

void device_create() {
  int ret = 0;
  memset(&device_inst, 0, sizeof(struct device));
  const struct config *conf = global_config_get();

  for (int i = 0; i < conf->nr_device; i++) {
    const struct device_desc *src_device_ref = &conf->devices[i];
    struct device_desc *dst_device_ref = &device_inst.device_descs[i];

    dst_device_ref->key = src_device_ref->key;
    snprintf(dst_device_ref->device_name, sizeof(dst_device_ref->device_name), "%s",
             src_device_ref->device_name);
    snprintf(dst_device_ref->device_type, sizeof(dst_device_ref->device_type), "%s",
             src_device_ref->device_type);

    if (strcmp(dst_device_ref->device_type, "dummy") == 0) {
      ret = netlink_add(dst_device_ref->device_type, dst_device_ref->device_name);
      if (ret != 0 && ret != -NLE_EXIST) {
        fprintf(stderr, "%s\n", nl_geterror(ret));
        goto failure;
      }
    } else if (strcmp(dst_device_ref->device_type, "tap") == 0) {
      ret = tap_alloc(dst_device_ref->device_name, IFF_TAP | IFF_NO_PI);
      if (ret <= 0) {
        perror("tap_alloc failed");
        goto failure;
      }
      device_inst.device_fds[i] = ret;
    } else {
      fprintf(stderr, "Currently, creation of this type of network card is not supported:%s",
              dst_device_ref->device_type);
    }
  }

  device_inst.nr_device = conf->nr_device;

end:
  return;

failure:
  exit(EXIT_FAILURE);
}

void device_index_get() {
  int ifindex = 0;
  for (unsigned int i = 0; i < device_inst.nr_device; i++) {
    const char *dev_name = device_inst.device_descs[i].device_name;

    ifindex = if_nametoindex(dev_name);
    if (ifindex == 0) {
      fprintf(stderr, "Failed to get %s index.", dev_name);
      goto failure;
    }

    device_inst.device_index[i] = ifindex;
  }

failure:
  exit(EXIT_FAILURE);
}

void device_open() {
  int ret = 0;
  for (unsigned int i = 0; i < device_inst.nr_device; i++) {
    const char *dev_name = device_inst.device_descs[i].device_name;
    ret = netlink_change(dev_name, IFF_UP);
    if (ret != 0) {
      fprintf(stderr, "Failed to start the device: %s", dev_name);
      exit(EXIT_FAILURE);
    }
  }
}

void device_init() {
  device_create();
  device_index_get();
  device_open();
}

const struct device *device_inst_get() { return &device_inst; }

int xdp_device_index_get() {
  for (unsigned int i = 0; i < device_inst.nr_device; i++) {
    const char *device_type = device_inst.device_descs[i].device_type;
    if (strcmp(device_type, "dummy") == 0) {
      return device_inst.device_index[i];
    }
  }

  return 0;
}