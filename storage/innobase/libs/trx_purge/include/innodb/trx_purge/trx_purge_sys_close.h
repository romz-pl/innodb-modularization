#pragma once

#include <innodb/univ/univ.h>

/** Frees the global purge system control structure. */
void trx_purge_sys_close(void);
