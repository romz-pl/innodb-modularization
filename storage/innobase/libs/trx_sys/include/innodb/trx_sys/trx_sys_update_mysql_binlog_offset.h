#pragma once

#include <innodb/univ/univ.h>

struct mtr_t;

/** Updates the offset information about the end of the MySQL binlog entry
 which corresponds to the transaction just being committed. In a MySQL
 replication slave updates the latest master binlog position up to which
 replication has proceeded. */
void trx_sys_update_mysql_binlog_offset(
    const char *file_name, /*!< in: MySQL log file name */
    int64_t offset,        /*!< in: position in that log file */
    ulint field,           /*!< in: offset of the MySQL log info field in
                           the trx sys header */
    mtr_t *mtr);           /*!< in: mtr */
