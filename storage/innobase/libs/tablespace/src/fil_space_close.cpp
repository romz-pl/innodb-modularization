#include <innodb/tablespace/fil_space_close.h>

#include <innodb/tablespace/fil_system.h>

/** Close each file of a tablespace if open.
@param[in]	space_id	tablespace identifier */
void fil_space_close(space_id_t space_id) {
  if (fil_system == nullptr) {
    return;
  }

  auto shard = fil_system->shard_by_id(space_id);

  shard->close_file(space_id);
}
