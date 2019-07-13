#include <innodb/tablespace/fil_space_get_flags.h>

#include <innodb/tablespace/fil_system.h>

/** Returns the flags of the space. The tablespace must be cached
in the memory cache.
@param[in]	space_id	Tablespace ID for which to get the flags
@return flags, ULINT_UNDEFINED if space not found */
uint32_t fil_space_get_flags(space_id_t space_id) {
  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->space_load(space_id);

  uint32_t flags;

  flags = (space != nullptr) ? space->flags : UINT32_UNDEFINED;

  shard->mutex_release();

  return (flags);
}
