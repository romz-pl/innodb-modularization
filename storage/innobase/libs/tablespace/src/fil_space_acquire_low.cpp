#include <innodb/tablespace/fil_space_acquire_low.h>

#include <innodb/tablespace/fil_system.h>

/** Acquire a tablespace when it could be dropped concurrently.
Used by background threads that do not necessarily hold proper locks
for concurrency control.
@param[in]	space_id	Tablespace ID
@param[in]	silent		Whether to silently ignore missing tablespaces
@return the tablespace, or nullptr if missing or being deleted */
fil_space_t *fil_space_acquire_low(space_id_t space_id, bool silent) {
  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->get_space_by_id(space_id);

  if (space == nullptr) {
    if (!silent) {
      ib::warn(ER_IB_MSG_286, ulong{space_id});
    }
  } else if (space->stop_new_ops) {
    space = nullptr;
  } else {
    ++space->n_pending_ops;
  }

  shard->mutex_release();

  return (space);
}
