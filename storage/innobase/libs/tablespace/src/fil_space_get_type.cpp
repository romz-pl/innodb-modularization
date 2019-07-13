#include <innodb/tablespace/fil_space_get_type.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG

/** Gets the type of a file space.
@param[in]	space_id	Tablespace ID
@return file type */
fil_type_t fil_space_get_type(space_id_t space_id) {
  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  auto space = shard->get_space_by_id(space_id);

  shard->mutex_release();

  return (space->purpose);
}

#endif
#endif
