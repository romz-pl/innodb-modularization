#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/memory/mem_heap_t.h>

struct dict_index_t;
struct dtuple_t;


/** Copies the first n fields of a physical record to a data tuple.
 The fields are copied to the memory heap. */
void rec_copy_prefix_to_dtuple(
    dtuple_t *tuple,           /*!< out: data tuple */
    const rec_t *rec,          /*!< in: physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint n_fields,            /*!< in: number of fields
                               to copy */
    mem_heap_t *heap);         /*!< in: memory heap */
