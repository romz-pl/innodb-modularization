#include <innodb/tablespace/fil_space_get_n_reserved_extents.h>

#include <innodb/tablespace/fil_system.h>

/** Gets the number of reserved extents. If the database is silent, this number
should be zero.
@param[in]	space_id	Tablespace ID
@return the number of reserved extents */
ulint fil_space_get_n_reserved_extents(space_id_t space_id) {
  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->get_space_by_id(space_id);

  ulint n = space->n_reserved_extents;

  shard->mutex_release();

  return (n);
}
