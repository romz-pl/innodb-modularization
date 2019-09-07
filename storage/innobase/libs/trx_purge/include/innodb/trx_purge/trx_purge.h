#pragma once

#include <innodb/univ/univ.h>

/** This function runs a purge batch.
 @return number of undo log pages handled in the batch */
ulint trx_purge(ulint n_purge_threads, /*!< in: number of purge tasks to
                                       submit to task queue. */
                ulint limit,           /*!< in: the maximum number of
                                       records to purge in one batch */
                bool truncate);        /*!< in: truncate history if true */
