#include <innodb/tablespace/fil_tablespace_open_for_recovery.h>

#include <innodb/tablespace/fil_system.h>

/** Open a tablespace that has a redo log record to apply.
@param[in]	space_id		Tablespace ID
@return true if the open was successful */
bool fil_tablespace_open_for_recovery(space_id_t space_id) {
  return (fil_system->open_for_recovery(space_id));
}
