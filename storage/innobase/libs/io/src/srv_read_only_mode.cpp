#include <innodb/io/srv_read_only_mode.h>

/** Set if InnoDB must operate in read-only mode. We don't do any
recovery and open all tables in RO mode instead of RW mode. We don't
sync the max trx id to disk either. */
bool srv_read_only_mode;
