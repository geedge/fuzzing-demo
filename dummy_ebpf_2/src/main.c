#include <argp.h>
#include <net/if.h>
#include <pthread.h>
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
#include "helper.h"
#include "tc_prog_user.h"

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
  struct bpf_object *obj = load_bpf_and_tc_attach();

  // User-level program writes forwarding rules into the map for use by kernel programs
  redirect_rule_set(obj);

  // read packet from tap
  pthread_t thread_id;
  int ret = pthread_create(&thread_id, NULL, data_poll, NULL);
  if (ret != 0) {
    printf("config reload thread failed : %s", strerror(ret));
    exit(EXIT_FAILURE);
  }

  // debug the output of the bpf_printk()
  // ret = pthread_create(&thread_id, NULL, read_trace_pipe, NULL);
  // if (ret != 0) {
  //   printf("config reload thread failed : %s", strerror(ret));
  //   exit(EXIT_FAILURE);
  // }

  while (!exiting) {
    sleep(1);
  }

cleanup:
  tc_dettach();
}