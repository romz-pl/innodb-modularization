#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/mlog_id_t.h>

struct mtr_t;

/** Writes 1, 2 or 4 bytes to a file page. Writes the corresponding log
 record to the mini-transaction log if mtr is not NULL. */
void mlog_write_ulint(
    byte *ptr,      /*!< in: pointer where to write */
    ulint val,      /*!< in: value to write */
    mlog_id_t type, /*!< in: MLOG_1BYTE, MLOG_2BYTES, MLOG_4BYTES */
    mtr_t *mtr);    /*!< in: mini-transaction handle */
