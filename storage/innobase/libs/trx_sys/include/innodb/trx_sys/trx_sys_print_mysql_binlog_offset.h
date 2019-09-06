#pragma once

#include <innodb/univ/univ.h>

/** Prints to stderr the MySQL binlog offset info in the trx system header if
 the magic number shows it valid. */
void trx_sys_print_mysql_binlog_offset(void);
