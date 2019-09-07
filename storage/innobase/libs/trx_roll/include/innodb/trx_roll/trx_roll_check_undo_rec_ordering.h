#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

bool trx_roll_check_undo_rec_ordering(
    undo_no_t curr_undo_rec_no,    /*!< in: record number of
                                   undo record to process. */
    space_id_t curr_undo_space_id, /*!< in: space-id of rollback
                                   segment that contains the
                                   undo record to process. */
    const trx_t *trx);              /*!< in: transaction */

#endif
