#include <innodb/tablespace/fsp_is_ibd_tablespace.h>

#include <innodb/tablespace/fsp_is_undo_tablespace.h>
#include <innodb/tablespace/fsp_is_system_temporary.h>
#include <innodb/tablespace/consts.h>

/** Determine if the space ID is an IBD tablespace, either file_per_table
or a general shared tablespace, where user tables exist.
@param[in]	space_id	tablespace ID
@return true if it is a user tablespace ID */
bool fsp_is_ibd_tablespace(space_id_t space_id) {
  return (space_id != TRX_SYS_SPACE && !fsp_is_undo_tablespace(space_id) &&
          !fsp_is_system_temporary(space_id));
}
