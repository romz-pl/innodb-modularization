#pragma once

#include <innodb/univ/univ.h>

/** Returns true if the record is of the insert type.
 @return true if the record was freshly inserted (not updated). */
bool trx_undo_trx_id_is_insert(
    const byte *trx_id) /*!< in: DB_TRX_ID, followed by DB_ROLL_PTR */
    MY_ATTRIBUTE((warn_unused_result));
