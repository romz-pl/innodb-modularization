#include <innodb/log_arch/srv_log_file_size.h>

/** At startup, this is the current redo log file size.
During startup, if this is different from srv_log_file_size_requested
(innodb_log_file_size), the redo log will be rebuilt and this size
will be initialized to srv_log_file_size_requested.
When upgrading from a previous redo log format, this will be set to 0,
and writing to the redo log is not allowed. Expressed in bytes. */
ulonglong srv_log_file_size;
