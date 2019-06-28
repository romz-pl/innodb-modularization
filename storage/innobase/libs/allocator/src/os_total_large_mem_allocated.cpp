#include <innodb/allocator/os_total_large_mem_allocated.h>

/** The total amount of memory currently allocated from the operating
system with os_mem_alloc_large(). */
ulint os_total_large_mem_allocated = 0;
