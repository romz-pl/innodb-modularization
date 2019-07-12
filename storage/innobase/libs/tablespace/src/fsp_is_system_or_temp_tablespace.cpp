#include <innodb/tablespace/fsp_is_system_or_temp_tablespace.h>

#include <innodb/tablespace/fsp_is_system_tablespace.h>
#include <innodb/tablespace/fsp_is_system_temporary.h>

/** Check if the space_id is for a system-tablespace (shared + temp).
@param[in]	space_id	tablespace ID
@return true if id is a system tablespace, false if not. */
bool fsp_is_system_or_temp_tablespace(space_id_t space_id) {
  return (fsp_is_system_tablespace(space_id) ||
          fsp_is_system_temporary(space_id));
}
