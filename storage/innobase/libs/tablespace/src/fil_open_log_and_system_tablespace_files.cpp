#include <innodb/tablespace/fil_open_log_and_system_tablespace_files.h>

#include <innodb/tablespace/fil_system.h>

/** Opens all log files and system tablespace data files. They stay open
until the database server shutdown. This should be called at a server
startup after the space objects for the log and the system tablespace
have been created. The purpose of this operation is to make sure we
never run out of file descriptors if we need to read from the insert
buffer or to write to the log. */
void fil_open_log_and_system_tablespace_files() {
  fil_system->open_all_system_tablespaces();
}
