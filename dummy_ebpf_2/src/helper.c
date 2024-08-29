#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "helper.h"

// from: https://github.com/xdp-project/bpf-examples/blob/master/ktrace-CO-RE/ktrace01.c

#define DEBUGFS "/sys/kernel/debug/tracing/"

void *read_trace_pipe(void *arg) {
  int trace_fd;

  trace_fd = open(DEBUGFS "trace_pipe", O_RDONLY, 0);
  if (trace_fd < 0)
    return NULL;

  while (1) {
    static char buf[4096];
    ssize_t sz;

    sz = read(trace_fd, buf, sizeof(buf) - 1);
    if (sz > 0) {
      buf[sz] = 0;
      puts(buf);
    }
  }

  return NULL;
}