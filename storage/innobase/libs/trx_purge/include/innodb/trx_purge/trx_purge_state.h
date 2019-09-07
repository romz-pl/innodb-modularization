#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_purge/purge_state_t.h>

/** Get the purge state.
 @return purge state. */
purge_state_t trx_purge_state(void);
