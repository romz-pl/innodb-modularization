#include <innodb/tablespace/fil_system_open_fetch.h>

#include <innodb/tablespace/fil_system.h>
#include <innodb/tablespace/dict_sys_t_is_reserved.h>

#ifdef INNODB_DD_TABLE
extern bool srv_is_upgrade_mode;
#endif /* INNODB_DD_TABLE */


/** Fetch the file name opened for a space_id during recovery
from the file map.
@param[in]	space_id	Undo tablespace ID
@return file name that was opened, empty string if space ID not found. */
std::string fil_system_open_fetch(space_id_t space_id) {
  ut_a(dict_sys_t_is_reserved(space_id) || srv_is_upgrade_mode);

  return (fil_system->find(space_id));
}
