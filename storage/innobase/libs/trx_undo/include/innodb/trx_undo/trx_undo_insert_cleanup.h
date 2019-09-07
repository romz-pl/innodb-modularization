#pragma once

#include <innodb/univ/univ.h>

struct trx_undo_ptr_t;

/** Frees an insert undo log after a transaction commit or rollback.
Knowledge of inserts is not needed after a commit or rollback, therefore
the data can be discarded.
@param[in,out]	undo_ptr	undo log to clean up
@param[in]	noredo		whether the undo tablespace is redo logged */
void trx_undo_insert_cleanup(trx_undo_ptr_t *undo_ptr, bool noredo);
