#include <innodb/tablespace/fil_truncate_tablespace.h>

#include <innodb/tablespace/fil_system.h>

/** Truncate the tablespace to needed size.
@param[in]	space_id	Tablespace ID to truncate
@param[in]	size_in_pages	Truncate size.
@return true if truncate was successful. */
bool fil_truncate_tablespace(space_id_t space_id, page_no_t size_in_pages) {
  auto shard = fil_system->shard_by_id(space_id);

  return (shard->space_truncate(space_id, size_in_pages));
}
