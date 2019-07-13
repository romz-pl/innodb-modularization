#include <innodb/tablespace/Fil_iterator.h>

#include <innodb/tablespace/fil_system.h>

/** Iterate through all persistent tablespace files (FIL_TYPE_TABLESPACE)
returning the nodes via callback function cbk.
@param[in]	include_log	include log files, if true
@param[in]	f		Callback
@return any error returned by the callback function. */
dberr_t Fil_iterator::iterate(bool include_log, Function &&f) {
  return (fil_system->iterate(include_log, f));
}
