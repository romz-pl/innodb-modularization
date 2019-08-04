#include <innodb/data_types/dtype_get_charset_coll.h>

#include <innodb/data_types/flags.h>

/** Gets the MySQL charset-collation code for MySQL string types.
 @return MySQL charset-collation code */
ulint dtype_get_charset_coll(ulint prtype) /*!< in: precise data type */
{
  return ((prtype >> 16) & CHAR_COLL_MASK);
}
