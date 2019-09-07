#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_undo_rec_t.h>

/** Reads the undo log record type.
 @return record type */
ulint trx_undo_rec_get_type(
    const trx_undo_rec_t *undo_rec); /*!< in: undo log record */
