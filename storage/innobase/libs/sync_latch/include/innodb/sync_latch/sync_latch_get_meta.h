#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_latch/latch_meta.h>

#ifndef UNIV_LIBRARY

/** Get the latch meta-data from the latch ID
@param[in]	id		Latch ID
@return the latch meta data */
inline latch_meta_t &sync_latch_get_meta(latch_id_t id) {
  ut_ad(static_cast<size_t>(id) < latch_meta.size());
  ut_ad(id == latch_meta[id]->get_id());

  return (*latch_meta[id]);
}

#endif
