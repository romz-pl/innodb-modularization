#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

struct dtuple_t;

/** Creates a data tuple with possible virtual columns to a memory heap.
@param[in]	heap		memory heap where the tuple is created
@param[in]	n_fields	number of fields
@param[in]	n_v_fields	number of fields on virtual col
@return own: created tuple */
dtuple_t *dtuple_create_with_vcol(mem_heap_t *heap, ulint n_fields,
                                  ulint n_v_fields);
