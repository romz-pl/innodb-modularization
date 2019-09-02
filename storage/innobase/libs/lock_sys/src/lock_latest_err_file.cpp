#include <innodb/lock_sys/lock_latest_err_file.h>

/** Only created if !srv_read_only_mode */
FILE *lock_latest_err_file;
