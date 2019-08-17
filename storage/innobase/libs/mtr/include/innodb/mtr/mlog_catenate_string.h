#pragma once

#include <innodb/univ/univ.h>

struct mtr_t;

/** Catenates n bytes to the mtr log. */
void mlog_catenate_string(mtr_t *mtr,      /*!< in: mtr */
                          const byte *str, /*!< in: string to write */
                          ulint len);      /*!< in: string length */
