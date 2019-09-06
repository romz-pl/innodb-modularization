#pragma once

#include <innodb/univ/univ.h>

#include "sql/handler.h"

struct trx_t;

/** This function is used to find one X/Open XA distributed transaction
 which is in the prepared state
 @return trx or NULL; on match, the trx->xid will be invalidated;
 note that the trx may have been committed, unless the caller is
 holding lock_sys->mutex */
trx_t *trx_get_trx_by_xid(
    const XID *xid); /*!< in: X/Open XA transaction identifier */
