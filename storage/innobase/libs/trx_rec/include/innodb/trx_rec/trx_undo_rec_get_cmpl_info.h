#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_undo_rec_t.h>

/** Reads from an undo log record the record compiler info.
 @return compiler info */
ulint trx_undo_rec_get_cmpl_info(
    const trx_undo_rec_t *undo_rec); /*!< in: undo log record */
