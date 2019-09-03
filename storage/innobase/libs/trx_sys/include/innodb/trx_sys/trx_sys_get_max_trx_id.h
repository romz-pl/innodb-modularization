#pragma once

#include <innodb/univ/univ.h>

/** Determines the maximum transaction id.
 @return maximum currently allocated trx id; will be stale after the
 next call to trx_sys_get_new_trx_id() */
trx_id_t trx_sys_get_max_trx_id(void);
