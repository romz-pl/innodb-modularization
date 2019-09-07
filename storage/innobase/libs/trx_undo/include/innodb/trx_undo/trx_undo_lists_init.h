#pragma once

#include <innodb/univ/univ.h>

struct trx_rseg_t;

/** Initializes the undo log lists for a rollback segment memory copy.
 This function is only called when the database is started or a new
 rollback segment created.
 @return the combined size of undo log segments in pages */
ulint trx_undo_lists_init(
    trx_rseg_t *rseg); /*!< in: rollback segment memory object */
