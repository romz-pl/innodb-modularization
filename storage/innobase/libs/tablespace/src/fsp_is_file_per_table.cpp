#include <innodb/tablespace/fsp_is_file_per_table.h>

#include <innodb/tablespace/fsp_is_ibd_tablespace.h>
#include <innodb/tablespace/fsp_is_shared_tablespace.h>

/** Check if tablespace is file-per-table.
@param[in]	space_id	tablespace ID
@param[in]	fsp_flags	tablespace flags
@return true if tablespace is file-per-table. */
bool fsp_is_file_per_table(space_id_t space_id, uint32_t fsp_flags) {
  return (!fsp_is_shared_tablespace(fsp_flags) &&
          fsp_is_ibd_tablespace(space_id));
}
