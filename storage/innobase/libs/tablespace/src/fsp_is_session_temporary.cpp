#include <innodb/tablespace/fsp_is_session_temporary.h>

#include <innodb/tablespace/consts.h>

/** Check if the tablespace is session temporary.
@param[in]      space_id        tablespace ID
@return true if tablespace is a session temporary tablespace. */
bool fsp_is_session_temporary(space_id_t space_id) {
  return (space_id > dict_sys_t_s_min_temp_space_id &&
          space_id <= dict_sys_t_s_max_temp_space_id);
}
