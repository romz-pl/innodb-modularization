#include <innodb/tablespace/dict_sys_t_is_reserved.h>

#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/fsp_is_session_temporary.h>

/** Check if a tablespace id is a reserved one
@param[in]	space	tablespace id to check
@return true if a reserved tablespace id, otherwise false */
bool dict_sys_t_is_reserved(space_id_t space) {
  return (space >= dict_sys_t_s_reserved_space_id ||
          fsp_is_session_temporary(space));
}
