#include <innodb/tablespace/fil_space_release_free_extents.h>

#include <innodb/tablespace/fil_system.h>

/** Releases free extents in a file space.
@param[in]	space_id	Tablespace ID
@param[in]	n_reserved	How many were reserved */
void fil_space_release_free_extents(space_id_t space_id, ulint n_reserved) {
  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->get_space_by_id(space_id);

  ut_a(n_reserved < std::numeric_limits<uint32_t>::max());
  ut_a(space->n_reserved_extents >= n_reserved);

  space->n_reserved_extents -= (uint32_t)n_reserved;

  shard->mutex_release();
}
