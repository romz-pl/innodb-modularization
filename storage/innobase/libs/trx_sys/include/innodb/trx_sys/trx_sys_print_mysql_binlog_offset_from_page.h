#pragma once

#include <innodb/univ/univ.h>

/** Prints to stderr the MySQL binlog info in the system header if the
 magic number shows it valid. */
void trx_sys_print_mysql_binlog_offset_from_page(
    const byte *page); /*!< in: buffer containing the trx
                       system header page, i.e., page number
                       TRX_SYS_PAGE_NO in the tablespace */
