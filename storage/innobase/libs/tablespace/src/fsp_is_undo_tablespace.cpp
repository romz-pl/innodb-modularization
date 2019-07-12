#include <innodb/tablespace/fsp_is_undo_tablespace.h>

#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/trx_sys_undo_spaces.h>

/** Check whether a space id is an undo tablespace ID
Undo tablespaces have space_id's starting 1 less than the redo logs.
They are numbered down from this.  Since rseg_id=0 always refers to the
system tablespace, undo_space_num values start at 1.  The current limit
is 127. The translation from an undo_space_num is:
   undo space_id = log_first_space_id - undo_space_num
@param[in]	space_id	space id to check
@return true if it is undo tablespace else false. */
bool fsp_is_undo_tablespace(space_id_t space_id) {
  /* Starting with v8, undo space_ids have a unique range. */
  if (space_id >= dict_sys_t_s_min_undo_space_id &&
      space_id <= dict_sys_t_s_max_undo_space_id) {
    return (true);
  }

  /* If upgrading from 5.7, there may be a list of old-style
  undo tablespaces.  Search them. */
  if (trx_sys_undo_spaces != nullptr) {
    return (trx_sys_undo_spaces->contains(space_id));
  }

  return (false);
}
