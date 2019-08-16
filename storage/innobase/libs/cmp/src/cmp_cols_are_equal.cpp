#include <innodb/cmp/cmp_cols_are_equal.h>

#include <innodb/dict_mem/dict_col_t.h>
#include <innodb/data_types/dtype_is_non_binary_string_type.h>
#include <innodb/data_types/dtype_get_charset_coll.h>
#include <innodb/data_types/dtype_is_binary_string_type.h>

/** Returns TRUE if two columns are equal for comparison purposes.
 @return true if the columns are considered equal in comparisons */
ibool cmp_cols_are_equal(const dict_col_t *col1, /*!< in: column 1 */
                         const dict_col_t *col2, /*!< in: column 2 */
                         ibool check_charsets)
/*!< in: whether to check charsets */
{
  if (dtype_is_non_binary_string_type(col1->mtype, col1->prtype) &&
      dtype_is_non_binary_string_type(col2->mtype, col2->prtype)) {
    /* Both are non-binary string types: they can be compared if
    and only if the charset-collation is the same */

    if (check_charsets) {
      return (dtype_get_charset_coll(col1->prtype) ==
              dtype_get_charset_coll(col2->prtype));
    } else {
      return (TRUE);
    }
  }

  if (dtype_is_binary_string_type(col1->mtype, col1->prtype) &&
      dtype_is_binary_string_type(col2->mtype, col2->prtype)) {
    /* Both are binary string types: they can be compared */

    return (TRUE);
  }

  if (col1->mtype != col2->mtype) {
    return (FALSE);
  }

  if (col1->mtype == DATA_INT &&
      (col1->prtype & DATA_UNSIGNED) != (col2->prtype & DATA_UNSIGNED)) {
    /* The storage format of an unsigned integer is different
    from a signed integer: in a signed integer we OR
    0x8000... to the value of positive integers. */

    return (FALSE);
  }

  return (col1->mtype != DATA_INT || col1->len == col2->len);
}
