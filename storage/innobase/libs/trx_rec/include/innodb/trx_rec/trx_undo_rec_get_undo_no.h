#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_undo_rec_t.h>

/** Reads the undo log record number.
 @return undo no */
undo_no_t trx_undo_rec_get_undo_no(
    const trx_undo_rec_t *undo_rec); /*!< in: undo log record */
