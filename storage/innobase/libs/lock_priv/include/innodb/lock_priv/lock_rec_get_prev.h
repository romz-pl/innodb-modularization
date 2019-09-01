#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

/** Gets the previous record lock set on a record.
 @return previous lock on the same record, NULL if none exists */
const lock_t *lock_rec_get_prev(
    const lock_t *in_lock, /*!< in: record lock */
    ulint heap_no);        /*!< in: heap number of the record */
