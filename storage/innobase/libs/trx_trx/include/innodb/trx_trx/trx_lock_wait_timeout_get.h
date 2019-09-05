#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>

ulong thd_lock_wait_timeout(THD *thd);

/**
Transactions that aren't started by the MySQL server don't set
the trx_t::mysql_thd field. For such transactions we set the lock
wait timeout to 0 instead of the user configured value that comes
from innodb_lock_wait_timeout via trx_t::mysql_thd.
@param	t transaction
@return lock wait timeout in seconds */
#define trx_lock_wait_timeout_get(t) thd_lock_wait_timeout((t)->mysql_thd)
