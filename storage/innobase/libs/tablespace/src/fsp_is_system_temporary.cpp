#include <innodb/tablespace/fsp_is_system_temporary.h>

#include <innodb/tablespace/fsp_is_global_temporary.h>
#include <innodb/tablespace/fsp_is_session_temporary.h>

/** Check if tablespace is system temporary.
@param[in]	space_id	tablespace ID
@return true if tablespace is system temporary. */
bool fsp_is_system_temporary(space_id_t space_id) {
  return (fsp_is_global_temporary(space_id) ||
          fsp_is_session_temporary(space_id));
}
