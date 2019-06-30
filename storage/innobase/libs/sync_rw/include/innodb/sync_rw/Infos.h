#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

#include <innodb/sync_rw/rw_lock_debug_t.h>
#include <vector>

/** For collecting the debug information for a thread's rw-lock */
typedef std::vector<rw_lock_debug_t *> Infos;


#endif
