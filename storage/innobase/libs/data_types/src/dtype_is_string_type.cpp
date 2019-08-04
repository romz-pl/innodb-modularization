#include <innodb/data_types/dtype_is_string_type.h>

#include <innodb/data_types/flags.h>

/** Checks if a data main type is a string type. Also a BLOB is considered a
 string type.
 @return true if string type */
ibool dtype_is_string_type(
    ulint mtype) /*!< in: InnoDB main data type code: DATA_CHAR, ... */
{
  if (mtype <= DATA_BLOB || mtype == DATA_MYSQL || mtype == DATA_VARMYSQL) {
    return (TRUE);
  }

  return (FALSE);
}
