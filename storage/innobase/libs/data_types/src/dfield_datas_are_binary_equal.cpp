#include <innodb/data_types/dfield_datas_are_binary_equal.h>

#include <innodb/data_types/dfield_t.h>

/** Tests if two data fields are equal.
 If len==0, tests the data length and content for equality.
 If len>0, tests the first len bytes of the content for equality.
 @return true if both fields are NULL or if they are equal */
ibool dfield_datas_are_binary_equal(
    const dfield_t *field1, /*!< in: field */
    const dfield_t *field2, /*!< in: field */
    ulint len)              /*!< in: maximum prefix to compare,
                            or 0 to compare the whole field length */
{
  ulint len2 = len;

  if (field1->len == UNIV_SQL_NULL || len == 0 || field1->len < len) {
    len = field1->len;
  }

  if (field2->len == UNIV_SQL_NULL || len2 == 0 || field2->len < len2) {
    len2 = field2->len;
  }

  return (len == len2 &&
          (len == UNIV_SQL_NULL || !memcmp(field1->data, field2->data, len)));
}
