#include <innodb/data_types/dtype_is_binary_string_type.h>

#include <innodb/data_types/flags.h>

/** Checks if a type is a binary string type. Note that for tables created with
 < 4.0.14, we do not know if a DATA_BLOB column is a BLOB or a TEXT column. For
 those DATA_BLOB columns this function currently returns FALSE.
 @return true if binary string type */
ibool dtype_is_binary_string_type(ulint mtype,  /*!< in: main data type */
                                  ulint prtype) /*!< in: precise type */
{
  if ((mtype == DATA_FIXBINARY) || (mtype == DATA_BINARY) ||
      (mtype == DATA_BLOB && (prtype & DATA_BINARY_TYPE))) {
    return (TRUE);
  }

  return (FALSE);
}
