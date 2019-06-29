#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_latch/latch_meta.h>
#include <innodb/sync_latch/sync_latch_get_meta.h>

#ifndef UNIV_LIBRARY

/** Fetch the counter for the latch
@param[in]	id		Latch ID
@return the latch counter */
inline latch_meta_t::CounterType *sync_latch_get_counter(latch_id_t id) {
  latch_meta_t &meta = sync_latch_get_meta(id);

  return (meta.get_counter());
}

#endif
