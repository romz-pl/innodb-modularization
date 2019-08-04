#include <innodb/data_types/dtype_form_prtype.h>

#include <innodb/assert/assert.h>
#include <innodb/data_types/flags.h>

/** Forms a precise type from the < 4.1.2 format precise type plus the
 charset-collation code.
 @return precise type, including the charset-collation code */
ulint dtype_form_prtype(
    ulint old_prtype,   /*!< in: the MySQL type code and the flags
                        DATA_BINARY_TYPE etc. */
    ulint charset_coll) /*!< in: MySQL charset-collation code */
{
  ut_a(old_prtype < 256 * 256);
  ut_a(charset_coll <= MAX_CHAR_COLL_NUM);

  return (old_prtype + (charset_coll << 16));
}
