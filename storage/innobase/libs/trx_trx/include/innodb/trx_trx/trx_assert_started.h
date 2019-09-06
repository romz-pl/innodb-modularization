#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

#ifdef UNIV_DEBUG
/** Asserts that a transaction has been started.
 The caller must hold trx_sys->mutex.
 @return true if started */
ibool trx_assert_started(const trx_t *trx) /*!< in: transaction */
    MY_ATTRIBUTE((warn_unused_result));
#endif /* UNIV_DEBUG */
