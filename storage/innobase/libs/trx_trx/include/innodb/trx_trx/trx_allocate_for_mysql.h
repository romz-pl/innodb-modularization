#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Creates a transaction object for MySQL.
 @return own: transaction object */
trx_t *trx_allocate_for_mysql(void);
