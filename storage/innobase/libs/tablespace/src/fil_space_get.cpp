#include <innodb/tablespace/fil_space_get.h>

#include <innodb/tablespace/fil_system.h>

/** Look up a tablespace. The caller should hold an InnoDB table lock or
a MDL that prevents the tablespace from being dropped during the operation,
or the caller should be in single-threaded crash recovery mode (no user
connections that could drop tablespaces). If this is not the case,
fil_space_acquire() and fil_space_release() should be used instead.
@param[in]	space_id	Tablespace ID
@return tablespace, or nullptr if not found */
fil_space_t *fil_space_get(space_id_t space_id) {
  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->get_space_by_id(space_id);

  shard->mutex_release();

  return (space);
}
