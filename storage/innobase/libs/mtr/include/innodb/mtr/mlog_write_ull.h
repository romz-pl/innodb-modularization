#pragma once

#include <innodb/univ/univ.h>

struct mtr_t;

/** Writes 8 bytes to a file page. Writes the corresponding log
 record to the mini-transaction log, only if mtr is not NULL */
void mlog_write_ull(byte *ptr,       /*!< in: pointer where to write */
                    ib_uint64_t val, /*!< in: value to write */
                    mtr_t *mtr);     /*!< in: mini-transaction handle */
