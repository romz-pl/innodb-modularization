#pragma once

#include <innodb/univ/univ.h>

/** Writes the value of max_trx_id to the file based trx system header. */
void trx_sys_flush_max_trx_id(void);
