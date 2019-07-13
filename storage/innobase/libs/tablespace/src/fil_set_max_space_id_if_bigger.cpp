#include <innodb/tablespace/fil_set_max_space_id_if_bigger.h>

#include <innodb/tablespace/fil_system.h>
#include <innodb/tablespace/dict_sys_t_is_reserved.h>
#include <innodb/logger/fatal.h>

/** Sets the max tablespace id counter if the given number is bigger than the
previous value.
@param[in]	max_id		Maximum known tablespace ID */
void fil_set_max_space_id_if_bigger(space_id_t max_id) {
  if (dict_sys_t_is_reserved(max_id)) {
    ib::fatal(ER_IB_MSG_285, ulong{max_id});
  }

  fil_system->update_maximum_space_id(max_id);
}
