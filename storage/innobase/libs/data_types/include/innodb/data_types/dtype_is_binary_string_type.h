#pragma once

#include <innodb/univ/univ.h>

/** Checks if a type is a binary string type. Note that for tables created with
 < 4.0.14, we do not know if a DATA_BLOB column is a BLOB or a TEXT column. For
 those DATA_BLOB columns this function currently returns FALSE.
 @return true if binary string type */
ibool dtype_is_binary_string_type(ulint mtype,   /*!< in: main data type */
                                  ulint prtype); /*!< in: precise type */
