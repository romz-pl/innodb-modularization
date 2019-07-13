#include <innodb/tablespace/fil_rename_tablespace.h>

#include <innodb/tablespace/fil_system.h>

/** Rename a single-table tablespace.
The tablespace must exist in the memory cache.
@param[in]	space_id	Tablespace ID
@param[in]	old_path	Old file name
@param[in]	new_name	New tablespace name in the schema/name format
@param[in]	new_path_in	New file name, or nullptr if it is located
                                in the normal data directory
@return InnoDB error code */
dberr_t fil_rename_tablespace(space_id_t space_id, const char *old_path,
                              const char *new_name, const char *new_path_in) {
  auto shard = fil_system->shard_by_id(space_id);

  dberr_t err = shard->space_rename(space_id, old_path, new_name, new_path_in);

  return (err);
}
