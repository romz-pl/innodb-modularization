#include <innodb/tablespace/fil_space_open.h>

#include <innodb/tablespace/fil_system.h>

/** Open each file of a tablespace if not already open.
@param[in]	space_id	tablespace identifier
@retval	true	if all file nodes were opened
@retval	false	on failure */
bool fil_space_open(space_id_t space_id) {
  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  bool success = shard->space_open(space_id);

  shard->mutex_release();

  return (success);
}
