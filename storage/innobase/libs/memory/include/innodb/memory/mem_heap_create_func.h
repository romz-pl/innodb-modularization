#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

mem_heap_t *mem_heap_create_func(ulint size,
#ifdef UNIV_DEBUG
                                 const char *file_name, ulint line,
#endif /* UNIV_DEBUG */
                                 ulint type);
