#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_latch/latch_meta.h>
#include <innodb/sync_latch/sync_latch_get_meta.h>

#ifndef UNIV_LIBRARY

/** Get the latch ordering level
@param[in]	id		Latch id to lookup
@return the latch level */
inline latch_level_t sync_latch_get_level(latch_id_t id) {
  const latch_meta_t &meta = sync_latch_get_meta(id);

  return (meta.get_level());
}

#endif

