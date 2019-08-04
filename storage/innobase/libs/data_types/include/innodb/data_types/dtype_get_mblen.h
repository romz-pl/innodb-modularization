#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

void dtype_get_mblen(ulint mtype,     /*!< in: main type */
                     ulint prtype,    /*!< in: precise type (and collation) */
                     ulint *mbminlen, /*!< out: minimum length of a
                                      multi-byte character */
                     ulint *mbmaxlen); /*!< out: maximum length of a
                                      multi-byte character */

#endif
