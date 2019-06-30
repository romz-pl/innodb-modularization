#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

void rw_lock_get_debug_info(const rw_lock_t *lock, Infos *infos);

#endif
