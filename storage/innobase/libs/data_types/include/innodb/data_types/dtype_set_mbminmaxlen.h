#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

struct dtype_t;

void dtype_set_mbminmaxlen(dtype_t *type,  /*!< in/out: type */
                           ulint mbminlen, /*!< in: minimum length of a char,
                                           in bytes, or 0 if this is not
                                           a character type */
                           ulint mbmaxlen); /*!< in: maximum length of a char,
                                           in bytes, or 0 if this is not
                                           a character type */

#endif
