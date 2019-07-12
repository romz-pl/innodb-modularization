#include <innodb/tablespace/fsp_is_global_temporary.h>

#include <innodb/tablespace/srv_tmp_space.h>


/** Check if tablespace is global temporary.
@param[in]	space_id	tablespace ID
@return true if tablespace is global temporary. */
bool fsp_is_global_temporary(space_id_t space_id) {
  return (space_id == srv_tmp_space.space_id());
}
