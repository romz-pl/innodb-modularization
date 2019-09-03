#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

/** Check if redo rseg is modified for insert/update. */
bool trx_is_redo_rseg_updated(const trx_t *trx);
