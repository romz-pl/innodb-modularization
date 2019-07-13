#include <innodb/tablespace/fil_delete_tablespace.h>

#include <innodb/tablespace/fil_system.h>

/** Deletes an IBD tablespace, either general or single-table.
The tablespace must be cached in the memory cache. This will delete the
datafile, fil_space_t & fil_node_t entries from the file_system_t cache.
@param[in]	space_id	Tablespace ID
@param[in]	buf_remove	Specify the action to take on the pages
                                for this table in the buffer pool.
@return DB_SUCCESS, DB_TABLESPCE_NOT_FOUND or DB_IO_ERROR */
dberr_t fil_delete_tablespace(space_id_t space_id, buf_remove_t buf_remove) {
  auto shard = fil_system->shard_by_id(space_id);

  return (shard->space_delete(space_id, buf_remove));
}
