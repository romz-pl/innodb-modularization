#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

struct dtuple_t;

/** Copies a data tuple to another.  This is a shallow copy; if a deep copy
 is desired, dfield_dup() will have to be invoked on each field.
 @return own: copy of tuple */
dtuple_t *dtuple_copy(const dtuple_t *tuple, /*!< in: tuple to copy from */
                      mem_heap_t *heap)      /*!< in: memory heap
                                             where the tuple is created */
    MY_ATTRIBUTE((malloc));
