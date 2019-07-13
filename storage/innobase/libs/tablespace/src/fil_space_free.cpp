#include <innodb/tablespace/fil_space_free.h>

#include <innodb/tablespace/fil_system.h>
#include <innodb/sync_rw/rw_lock_x_unlock.h>

/** Frees a space object from the tablespace memory cache.
Closes a tablespaces' files but does not delete them.
There must not be any pending i/o's or flushes on the files.
@param[in]	space_id	Tablespace ID
@param[in]	x_latched	Whether the caller holds X-mode space->latch
@return true if success */
bool fil_space_free(space_id_t space_id, bool x_latched) {
  ut_ad(space_id != TRX_SYS_SPACE);

  auto shard = fil_system->shard_by_id(space_id);
  auto space = shard->space_free(space_id);

  if (space == nullptr) {
    return (false);
  }

  if (x_latched) {
    rw_lock_x_unlock(&space->latch);
  }

  Fil_shard::space_free_low(space);
  ut_a(space == nullptr);

  return (true);
}
