#pragma once

#include <innodb/univ/univ.h>

/** Allocates a new transaction id.
 @return new, allocated trx id */
trx_id_t trx_sys_get_new_trx_id();
