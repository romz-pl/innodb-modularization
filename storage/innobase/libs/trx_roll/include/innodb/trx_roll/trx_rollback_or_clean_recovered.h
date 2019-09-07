#pragma once

#include <innodb/univ/univ.h>

/** Rollback or clean up any incomplete transactions which were
 encountered in crash recovery.  If the transaction already was
 committed, then we clean up a possible insert undo log. If the
 transaction was not yet committed, then we roll it back. */
void trx_rollback_or_clean_recovered(
    ibool all); /*!< in: FALSE=roll back dictionary transactions;
                TRUE=roll back all non-PREPARED transactions */
