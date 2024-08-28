#include <argp.h>
#include <net/if.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <xdp/libxdp.h>

#include "config.h"
#include "data.h"
#include "device.h"
#include "xdp_prog_user.h"

volatile bool exiting = false;
static void sig_handler(int sig) { exiting = true; }

int main(int argc, char *argv[]) {
  // init config
  global_config_init();

  // create and open device
  device_init();

  /* Cleaner handling of Ctrl-C */
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);

  // load ebpf prog
  struct xdp_program *prog = load_bpf_and_xdp_attach();

  // User-level program writes forwarding rules into the map for use by kernel programs
  redirect_rule_set(prog);

  // read packet from tap
  data_poll();

cleanup:
  int ifindex = xdp_device_index_get();
  struct xdp_multiprog *mp = xdp_multiprog__get_from_ifindex(ifindex);
  int ret = xdp_multiprog__detach(mp);
  if (ret != 0) {
    perror("xdp_multiprog__detach failed.");
    exit(EXIT_FAILURE);
  }
}