#include <innodb/tablespace/fsp_is_checksum_disabled.h>

#include <innodb/tablespace/fsp_is_system_temporary.h>

/** Check if checksum is disabled for the given space.
@param[in]	space_id	tablespace ID
@return true if checksum is disabled for given space. */
bool fsp_is_checksum_disabled(space_id_t space_id) {
  return (fsp_is_system_temporary(space_id));
}
