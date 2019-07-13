#include <innodb/tablespace/fil_space_release.h>

#include <innodb/tablespace/fil_system.h>

/** Release a tablespace acquired with fil_space_acquire().
@param[in,out]	space	tablespace to release  */
void fil_space_release(fil_space_t *space) {
  auto shard = fil_system->shard_by_id(space->id);

  shard->mutex_acquire();

  ut_ad(space->magic_n == FIL_SPACE_MAGIC_N);
  ut_ad(space->n_pending_ops > 0);

  --space->n_pending_ops;

  shard->mutex_release();
}
