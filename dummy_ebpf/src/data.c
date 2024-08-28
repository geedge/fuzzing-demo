#include "data.h"
#include "device.h"

#include <xdp/parsing_helpers.h>

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

extern volatile bool exiting;

void print_mac_address(unsigned char *mac, char *label) {
  printf("%s: %02x:%02x:%02x:%02x:%02x:%02x\n", label, mac[0], mac[1], mac[2], mac[3], mac[4],
         mac[5]);
}

void packet_parse(char *buf, int size) {
  struct hdr_cursor nh;
  struct ethhdr *eth;
  struct iphdr *iphdr;
  struct ipv6hdr *ipv6hdr;
  struct udphdr *udphdr;
  struct tcphdr *tcphdr;

  nh.pos = buf;
  void *data_end = (void *)(buf + size);
  parse_ethhdr(&nh, data_end, &eth);

  print_mac_address(eth->h_source, "    Source MAC");
  print_mac_address(eth->h_dest, "  Destination MAC");

  return;
}

void read_process(int fd) {
#define PACKET_MAX_LEN 2048
  char buf[PACKET_MAX_LEN] = {0};

  int n = read(fd, buf, sizeof(buf));
  if (n == -1 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
    return;
  } else if (n < 0) {
    printf("read_process failed: %s\n", strerror(errno));
    return;
  }

  const struct device_desc *device_desc_ref = device_desc_find_by_fd(fd);
  printf("%s recv a packet.", device_desc_ref->device_name);

  packet_parse(buf, n);
}

void data_poll() {
  const struct device *device_inst = device_inst_get();

  int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
  if (epoll_fd < 0) {
    printf("Cannot create epoll fd: %s\n", strerror(errno));
    goto failure;
  }

  for (int i = 0; i < device_inst->nr_device; i++) {
    int device_fd = device_inst->device_fds[i];
    if (device_fd > 0) {
      struct epoll_event event = {};
      event.data.fd = device_fd;
      event.events = EPOLLIN;

      if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, device_fd, &event)) {
        printf("update epoll event failed : %s\n", strerror(errno));
        return;
      }
    }
  }

  struct epoll_event evlist[DEVICE_MAX_NUM];
  while (!exiting) {
    int ret = epoll_wait(epoll_fd, evlist, DEVICE_MAX_NUM, -1);
    if (ret == -1 && errno == EINTR)
      continue;
    else if (ret == -1) {
      printf("loop thread failed : %s", strerror(errno));
      goto failure;
    }

    for (int i = 0; i < ret; i++) {
      int fd = evlist[i].data.fd;

      if (evlist[i].events & EPOLLIN) {
        read_process(fd);
      } else {
        printf("Unhandled epoll type:%d", evlist[i].events);
      }
    }
  }

  return;

failure:
  exit(EXIT_FAILURE);
}