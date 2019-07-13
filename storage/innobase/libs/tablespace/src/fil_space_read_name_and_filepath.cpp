#include <innodb/tablespace/fil_space_read_name_and_filepath.h>

#include <innodb/tablespace/fil_system.h>
#include <innodb/string/mem_strdup.h>

/** Looks for a pre-existing fil_space_t with the given tablespace ID
and, if found, returns the name and filepath in newly allocated buffers
that the caller must free.
@param[in]	space_id	The tablespace ID to search for.
@param[out]	name		Name of the tablespace found.
@param[out]	filepath	The filepath of the first datafile for the
tablespace.
@return true if tablespace is found, false if not. */
bool fil_space_read_name_and_filepath(space_id_t space_id, char **name,
                                      char **filepath) {
  bool success = false;

  *name = nullptr;
  *filepath = nullptr;

  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->get_space_by_id(space_id);

  if (space != nullptr) {
    *name = mem_strdup(space->name);

    *filepath = mem_strdup(space->files.front().name);

    success = true;
  }

  shard->mutex_release();

  return (success);
}
