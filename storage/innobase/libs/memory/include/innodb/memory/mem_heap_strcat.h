#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

/** Concatenate two strings and return the result, using a memory heap.
 @return own: the result */
char *mem_heap_strcat(
    mem_heap_t *heap, /*!< in: memory heap where string is allocated */
    const char *s1,   /*!< in: string 1 */
    const char *s2);  /*!< in: string 2 */
