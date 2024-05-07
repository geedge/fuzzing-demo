#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define DP_TRACE_INFO(mr_ins, mr_buff, module, fmt, ...)                       \
  do {                                                                         \
    if ((mr_ins) == 0) {                                                       \
      printf("1\n");                                                           \
    };                                                                         \
    if ((mr_ins) == 1) {                                                       \
      printf("2\n");                                                           \
    }                                                                          \
  } while (0)

int main(int argc, char *argv[]) { DP_TRACE_INFO(1, 0, 0, 0, 0, 0); }