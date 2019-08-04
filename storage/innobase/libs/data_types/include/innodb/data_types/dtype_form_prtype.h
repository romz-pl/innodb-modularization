#pragma once

#include <innodb/univ/univ.h>

/** Forms a precise type from the < 4.1.2 format precise type plus the
 charset-collation code.
 @return precise type, including the charset-collation code */
ulint dtype_form_prtype(
    ulint old_prtype,    /*!< in: the MySQL type code and the flags
                         DATA_BINARY_TYPE etc. */
    ulint charset_coll); /*!< in: MySQL charset-collation code */
