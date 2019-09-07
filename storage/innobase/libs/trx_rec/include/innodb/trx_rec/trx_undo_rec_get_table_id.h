#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_undo_rec_t.h>

/** Reads from an undo log record the table ID
@param[in]	undo_rec	Undo log record
@return the table ID */
table_id_t trx_undo_rec_get_table_id(const trx_undo_rec_t *undo_rec)
    MY_ATTRIBUTE((warn_unused_result));
