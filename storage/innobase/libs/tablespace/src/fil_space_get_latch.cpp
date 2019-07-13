#include <innodb/tablespace/fil_space_get_latch.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/tablespace/fil_system.h>

/** Returns the latch of a file space.
@param[in]	space_id	Tablespace ID
@return latch protecting storage allocation */
rw_lock_t *fil_space_get_latch(space_id_t space_id) {
  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->get_space_by_id(space_id);

  shard->mutex_release();

  return (&space->latch);
}

#endif
