#pragma once

#include <innodb/univ/univ.h>
#include <innodb/univ/rec_t.h>

struct buf_block_t;

/** Restores the state of explicit lock requests on a single record, where the
 state was stored on the infimum of the page. */
void lock_rec_restore_from_page_infimum(
    const buf_block_t *block,    /*!< in: buffer block containing rec */
    const rec_t *rec,            /*!< in: record whose lock state
                                 is restored */
    const buf_block_t *donator); /*!< in: page (rec is not
                                necessarily on this page)
                                whose infimum stored the lock
                                state; lock bits are reset on
                                the infimum */
