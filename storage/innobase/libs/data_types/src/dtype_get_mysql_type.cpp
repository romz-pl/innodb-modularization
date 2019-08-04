#include <innodb/data_types/dtype_get_mysql_type.h>

#include <innodb/data_types/dtype_t.h>

/** Gets the MySQL type code from a dtype.
 @return MySQL type code; this is NOT an InnoDB type code! */
ulint dtype_get_mysql_type(const dtype_t *type) /*!< in: type struct */
{
  return (type->prtype & 0xFFUL);
}
