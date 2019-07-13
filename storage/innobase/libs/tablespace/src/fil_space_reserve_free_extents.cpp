#include <innodb/tablespace/fil_space_reserve_free_extents.h>

#include <innodb/tablespace/fil_system.h>

/** Tries to reserve free extents in a file space.
@param[in]	space_id	Tablespace ID
@param[in]	n_free_now	Number of free extents now
@param[in]	n_to_reserve	How many one wants to reserve
@return true if succeed */
bool fil_space_reserve_free_extents(space_id_t space_id, ulint n_free_now,
                                    ulint n_to_reserve) {
  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->get_space_by_id(space_id);

  bool success;

  if (space->n_reserved_extents + n_to_reserve > n_free_now) {
    success = false;
  } else {
    ut_a(n_to_reserve < std::numeric_limits<uint32_t>::max());
    space->n_reserved_extents += (uint32_t)n_to_reserve;
    success = true;
  }

  shard->mutex_release();

  return (success);
}
