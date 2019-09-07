#pragma once

#include <innodb/univ/univ.h>

/** Stop purge and wait for it to stop, move to PURGE_STATE_STOP. */
void trx_purge_stop(void);
