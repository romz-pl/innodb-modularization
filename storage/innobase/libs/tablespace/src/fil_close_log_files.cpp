#include <innodb/tablespace/fil_close_log_files.h>

#include <innodb/tablespace/fil_system.h>

/** Closes the redo log files. There must not be any pending i/o's or not
flushed modifications in the files.
@param[in]	free_all	If set then free all instances */
void fil_close_log_files(bool free_all) {
  fil_system->close_all_log_files(free_all);
}
