#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_latch/latch_meta.h>
#include <innodb/sync_latch/sync_latch_get_meta.h>

#ifndef UNIV_LIBRARY

/** Get the latch name from the latch ID
@param[in]	id		Latch ID
@return the name, will assert if not found */
inline const char *sync_latch_get_name(latch_id_t id) {
  const latch_meta_t &meta = sync_latch_get_meta(id);

  return (meta.get_name());
}

#ifdef UNIV_PFS_MUTEX
#ifndef UNIV_HOTBACKUP

/** Get the latch name from a sync level
@param[in]	level		Latch level to lookup
@return 0 if not found. */
const char *sync_latch_get_name(latch_level_t level);

#endif
#endif
#endif
