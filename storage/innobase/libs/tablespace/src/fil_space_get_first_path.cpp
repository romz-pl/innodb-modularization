#include <innodb/tablespace/fil_space_get_first_path.h>

#include <innodb/tablespace/fil_system.h>
#include <innodb/string/mem_strdup.h>

/** Returns the path from the first fil_node_t found with this space ID.
The caller is responsible for freeing the memory allocated here for the
value returned.
@param[in]	space_id	Tablespace ID
@return own: A copy of fil_node_t::path, nullptr if space ID is zero
or not found. */
char *fil_space_get_first_path(space_id_t space_id) {
  ut_a(space_id != TRX_SYS_SPACE);

  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->space_load(space_id);

  char *path;

  if (space != nullptr) {
    path = mem_strdup(space->files.front().name);
  } else {
    path = nullptr;
  }

  shard->mutex_release();

  return (path);
}

