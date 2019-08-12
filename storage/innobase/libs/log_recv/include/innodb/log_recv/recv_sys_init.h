#pragma once

#include <innodb/univ/univ.h>

/** Inits the recovery system for a recovery operation.
@param[in]	max_mem		Available memory in bytes */
void recv_sys_init(ulint max_mem);
