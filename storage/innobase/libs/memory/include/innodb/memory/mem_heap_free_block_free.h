#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

#ifndef UNIV_HOTBACKUP
#ifndef UNIV_LIBRARY

/** Frees the free_block field from a memory heap. */
void mem_heap_free_block_free(mem_heap_t *heap); /*!< in: heap */

#endif                                           /* !UNIV_LIBRARY */
#endif                                           /* !UNIV_HOTBACKUP */


