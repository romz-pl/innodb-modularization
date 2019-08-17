#pragma once

#include <innodb/univ/univ.h>

struct mtr_t;

/** Writes a string to a file page buffered in the buffer pool. Writes the
 corresponding log record to the mini-transaction log. */
void mlog_write_string(byte *ptr,       /*!< in: pointer where to write */
                       const byte *str, /*!< in: string to write */
                       ulint len,       /*!< in: string length */
                       mtr_t *mtr);     /*!< in: mini-transaction handle */
