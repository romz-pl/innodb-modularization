#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

mem_block_t *mem_heap_create_block_func(
    mem_heap_t *heap, /*!< in: memory heap or NULL if first block
                      should be created */
    ulint n,          /*!< in: number of bytes needed for user data */
#ifdef UNIV_DEBUG
    const char *file_name, /*!< in: file name where created */
    ulint line,            /*!< in: line where created */
#endif                     /* UNIV_DEBUG */
    ulint type);            /*!< in: type of heap: MEM_HEAP_DYNAMIC or
                           MEM_HEAP_BUFFER */
