#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

struct trx_t;
struct trx_undo_ptr_t;

/** Assigns an undo log for a transaction. A new undo log is created or a cached
 undo log reused.
 @return DB_SUCCESS if undo log assign successful, possible error codes
 are: DB_TOO_MANY_CONCURRENT_TRXS DB_OUT_OF_FILE_SPACE DB_READ_ONLY
 DB_OUT_OF_MEMORY */
dberr_t trx_undo_assign_undo(
    trx_t *trx,               /*!< in: transaction */
    trx_undo_ptr_t *undo_ptr, /*!< in: assign undo log from
                              referred rollback segment. */
    ulint type)               /*!< in: TRX_UNDO_INSERT or
                              TRX_UNDO_UPDATE */
    MY_ATTRIBUTE((warn_unused_result));
