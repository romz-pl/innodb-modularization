#include <innodb/data_types/dfield_data_is_binary_equal.h>

#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dfield_get_data.h>

/** Tests if dfield data length and content is equal to the given.
 @return true if equal */
ibool dfield_data_is_binary_equal(
    const dfield_t *field, /*!< in: field */
    ulint len,             /*!< in: data length or UNIV_SQL_NULL */
    const byte *data)      /*!< in: data */
{
  return (len == dfield_get_len(field) &&
          (len == UNIV_SQL_NULL || len == 0 ||
           !memcmp(dfield_get_data(field), data, len)));
}
