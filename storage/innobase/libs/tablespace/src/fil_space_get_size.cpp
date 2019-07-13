#include <innodb/tablespace/fil_space_get_size.h>

#include <innodb/tablespace/fil_system.h>

/** Returns the size of the space in pages. The tablespace must be cached
in the memory cache.
@param[in]	space_id	Tablespace ID
@return space size, 0 if space not found */
page_no_t fil_space_get_size(space_id_t space_id) {
  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->space_load(space_id);

  page_no_t size = space ? space->size : 0;

  shard->mutex_release();

  return (size);
}
