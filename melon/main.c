#include "mln_alloc.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  char *ptr;
  mln_alloc_t *pool;

  pool = mln_alloc_init(NULL);

  ptr = mln_alloc_m(pool, 1024);
  printf("%p\n", ptr);
  mln_alloc_free(ptr);

  mln_alloc_destroy(pool);
  return 0;
}