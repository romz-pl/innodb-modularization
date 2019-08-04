#pragma once

#include <innodb/univ/univ.h>

/** Get the variable length bounds of the given character set. */
void innobase_get_cset_width(
    ulint cset,       /*!< in: MySQL charset-collation code */
    ulint *mbminlen,  /*!< out: minimum length of a char (in bytes) */
    ulint *mbmaxlen); /*!< out: maximum length of a char (in bytes) */
