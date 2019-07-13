#include <innodb/tablespace/fil_space_extend.h>

#include <innodb/tablespace/fil_system.h>

/** Try to extend a tablespace if it is smaller than the specified size.
@param[in,out]	space	tablespace
@param[in]	size	desired size in pages
@return whether the tablespace is at least as big as requested */
bool fil_space_extend(fil_space_t *space, page_no_t size) {
  auto shard = fil_system->shard_by_id(space->id);

  return (shard->space_extend(space, size));
}
