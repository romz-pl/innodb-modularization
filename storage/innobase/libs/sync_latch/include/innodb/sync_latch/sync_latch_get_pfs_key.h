#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_latch/sync_latch_get_meta.h>
#include <innodb/pfs/mysql_pfs_key_t.h>

#ifndef UNIV_LIBRARY
#ifdef UNIV_PFS_MUTEX

/** Get the latch PFS key from the latch ID
@param[in]	id		Latch ID
@return the PFS key */
inline mysql_pfs_key_t sync_latch_get_pfs_key(latch_id_t id) {
  const latch_meta_t &meta = sync_latch_get_meta(id);

  return (meta.get_pfs_key());
}

#endif
#endif
