#include <innodb/tablespace/fil_space_update_name.h>

#include <innodb/tablespace/fil_space_t.h>
#include <innodb/tablespace/fil_rename_tablespace_by_id.h>
#include <innodb/logger/warn.h>

#include <string.h>

/** Update the tablespace name. Incase, the new name
and old name are same, no update done.
@param[in,out]	space		tablespace object on which name
                                will be updated
@param[in]	name		new name for tablespace */
void fil_space_update_name(fil_space_t *space, const char *name) {
  if (space == nullptr || name == nullptr || space->name == nullptr ||
      strcmp(space->name, name) == 0) {
    return;
  }

  dberr_t err = fil_rename_tablespace_by_id(space->id, space->name, name);

  if (err != DB_SUCCESS) {
    ib::warn(ER_IB_MSG_387) << "Tablespace rename '" << space->name << "' to"
                            << " '" << name << "' failed!";
  }
}
