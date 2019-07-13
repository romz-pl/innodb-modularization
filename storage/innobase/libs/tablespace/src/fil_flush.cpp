#include <innodb/tablespace/fil_flush.h>

#include <innodb/tablespace/fil_system.h>

/** Flushes to disk possible writes cached by the OS. If the space does
not exist or is being dropped, does not do anything.
@param[in]	space_id	File space ID (this can be a group of log files
                                or a tablespace of the database) */
void fil_flush(space_id_t space_id) {
  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  /* Note: Will release and reacquire the Fil_shard::mutex. */
  shard->space_flush(space_id);

  shard->mutex_release();
}
