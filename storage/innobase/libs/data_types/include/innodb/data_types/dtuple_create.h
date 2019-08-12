#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

struct dtuple_t;

/** Creates a data tuple to a memory heap. The default value for number
 of fields used in record comparisons for this tuple is n_fields.
 @return own: created tuple */
dtuple_t *dtuple_create(
    mem_heap_t *heap, /*!< in: memory heap where the tuple
                      is created, DTUPLE_EST_ALLOC(n_fields)
                      bytes will be allocated from this heap */
    ulint n_fields)   /*!< in: number of fields */
    MY_ATTRIBUTE((malloc));
