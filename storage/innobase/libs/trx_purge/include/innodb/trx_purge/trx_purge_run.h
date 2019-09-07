#pragma once

#include <innodb/univ/univ.h>

/** Resume purge, move to PURGE_STATE_RUN. */
void trx_purge_run(void);
