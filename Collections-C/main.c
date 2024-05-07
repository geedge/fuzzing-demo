/* CC_StaticPool can enable the use of the structures on the stack */

#include "cc_list.h"
#include "memory/cc_dynamic_pool.h"

int main(int argc, char **argv) {
  CC_DynamicPool *pool;
  cc_dynamic_pool_new(1024 * 1024, &pool);

  CC_ListConf conf; /* Create a new list config */
  cc_list_conf_init(&conf);
  conf.mem_alloc =
      pool_malloc; /* Set list memory allocators to pool allocators */
  conf.mem_calloc = pool_calloc;
  conf.mem_free = pool_free;

  CC_List *list;
  cc_list_new_conf(&conf, &list); /* The newly created list will be allocated
                                     inside the "buffer" array*/

  // Use the list

  return 0;
}