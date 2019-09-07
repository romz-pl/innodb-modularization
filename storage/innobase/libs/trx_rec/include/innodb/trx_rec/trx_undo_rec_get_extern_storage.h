#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_undo_rec_t.h>

/** Returns TRUE if an undo log record contains an extern storage field.
 @return true if extern */
ibool trx_undo_rec_get_extern_storage(
    const trx_undo_rec_t *undo_rec); /*!< in: undo log record */
