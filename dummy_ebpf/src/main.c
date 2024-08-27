#include <argp.h>
#include <net/if.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "device.h"
#include "xdp_prog_user.h"

static volatile bool exiting = false;
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
  load_bpf_and_xdp_attach();
}