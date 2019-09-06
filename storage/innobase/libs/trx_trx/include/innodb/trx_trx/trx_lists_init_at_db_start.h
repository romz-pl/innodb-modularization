#pragma once

#include <innodb/univ/univ.h>

/** Creates trx objects for transactions and initializes the trx list of
 trx_sys at database start. Rollback segment and undo log lists must
 already exist when this function is called, because the lists of
 transactions to be rolled back or cleaned up are built based on the
 undo log lists. */
void trx_lists_init_at_db_start(void);
