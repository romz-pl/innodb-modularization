#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Creates a transaction object for background operations by the master thread.
 @return own: transaction object */
trx_t *trx_allocate_for_background(void);
