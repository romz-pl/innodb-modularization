#pragma once

#include <innodb/univ/univ.h>

struct buf_block_t;

/** Resets the original locks on heir and replaces them with gap type locks
 inherited from rec. */
void lock_rec_reset_and_inherit_gap_locks(
    const buf_block_t *heir_block, /*!< in: block containing the
                                   record which inherits */
    const buf_block_t *block,      /*!< in: block containing the
                                   record from which inherited;
                                   does NOT reset the locks on
                                   this record */
    ulint heir_heap_no,            /*!< in: heap_no of the
                                   inheriting record */
    ulint heap_no);                /*!< in: heap_no of the
                                   donating record */
