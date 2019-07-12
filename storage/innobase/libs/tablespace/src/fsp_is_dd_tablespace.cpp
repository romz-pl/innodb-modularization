#include <innodb/tablespace/fsp_is_dd_tablespace.h>

#include <innodb/tablespace/consts.h>

/** Check if tablespace is dd tablespace.
@param[in]      space_id        tablespace ID
@return true if tablespace is dd tablespace. */
bool fsp_is_dd_tablespace(space_id_t space_id) {
  return (space_id == dict_sys_t_s_space_id);
}
