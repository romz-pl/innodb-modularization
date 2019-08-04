#pragma once

#include <innodb/univ/univ.h>

/** Checks if a type is a non-binary string type. That is, dtype_is_string_type
 is TRUE and dtype_is_binary_string_type is FALSE. Note that for tables created
 with < 4.0.14, we do not know if a DATA_BLOB column is a BLOB or a TEXT column.
 For those DATA_BLOB columns this function currently returns TRUE.
 @return true if non-binary string type */
ibool dtype_is_non_binary_string_type(ulint mtype,   /*!< in: main data type */
                                      ulint prtype); /*!< in: precise type */
