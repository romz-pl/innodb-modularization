#pragma once

#include <innodb/univ/univ.h>

struct mtr_t;

/** Logs a write of a string to a file page buffered in the buffer pool.
 Writes the corresponding log record to the mini-transaction log. */
void mlog_log_string(byte *ptr,   /*!< in: pointer written to */
                     ulint len,   /*!< in: string length */
                     mtr_t *mtr); /*!< in: mini-transaction handle */

