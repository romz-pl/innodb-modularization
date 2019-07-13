#include <innodb/tablespace/fil_rename_tablespace_by_id.h>

#include <innodb/tablespace/fil_system.h>

/* Rename a tablespace.  Use the space_id to find the shard.
@param[in]	space_id	tablespace ID
@param[in]	old_name	old tablespace name
@param[in]	new_name	new tablespace name
@return DB_SUCCESS on success */
dberr_t fil_rename_tablespace_by_id(space_id_t space_id, const char *old_name,
                                    const char *new_name) {
  return (fil_system->rename_tablespace_name(space_id, old_name, new_name));
}
