#pragma once

#include <innodb/univ/univ.h>

/** Creates the trx_sys instance and initializes purge_queue and mutex. */
void trx_sys_create(void);
