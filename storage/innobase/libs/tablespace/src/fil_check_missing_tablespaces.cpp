#include <innodb/tablespace/fil_check_missing_tablespaces.h>

#include <innodb/tablespace/fil_system.h>

/** This function should be called after recovery has completed.
Check for tablespace files for which we did not see any MLOG_FILE_DELETE
or MLOG_FILE_RENAME record. These could not be recovered
@return true if there were some filenames missing for which we had to
        ignore redo log records during the apply phase */
bool fil_check_missing_tablespaces() {
  return (fil_system->check_missing_tablespaces());
}
