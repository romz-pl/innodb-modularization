#include <innodb/tablespace/fil_rename_tablespace_check.h>

#include <innodb/io/os_file_status.h>
#include <innodb/logger/error.h>

/** Test if a tablespace file can be renamed to a new filepath by checking
if that the old filepath exists and the new filepath does not exist.
@param[in]	space_id	tablespace id
@param[in]	old_path	old filepath
@param[in]	new_path	new filepath
@param[in]	is_discarded	whether the tablespace is discarded
@return innodb error code */
dberr_t fil_rename_tablespace_check(space_id_t space_id, const char *old_path,
                                    const char *new_path, bool is_discarded) {
  bool exists = false;
  os_file_type_t ftype;

  if (!is_discarded && os_file_status(old_path, &exists, &ftype) && !exists) {
    ib::error(ER_IB_MSG_293, old_path, new_path, ulong{space_id});
    return (DB_TABLESPACE_NOT_FOUND);
  }

  exists = false;

  if (!os_file_status(new_path, &exists, &ftype) || exists) {
    ib::error(ER_IB_MSG_294, old_path, new_path, ulong{space_id});
    return (DB_TABLESPACE_EXISTS);
  }

  return (DB_SUCCESS);
}
