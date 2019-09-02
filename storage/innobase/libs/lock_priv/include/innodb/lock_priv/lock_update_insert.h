#pragma once

#include <innodb/univ/univ.h>
#include <innodb/univ/rec_t.h>

struct buf_block_t;

/** Updates the lock table when a new user record is inserted. */
void lock_update_insert(
    const buf_block_t *block, /*!< in: buffer block containing rec */
    const rec_t *rec);        /*!< in: the inserted record */
