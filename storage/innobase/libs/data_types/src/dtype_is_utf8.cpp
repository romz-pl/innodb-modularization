#include <innodb/data_types/dtype_is_utf8.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/data_types/dtype_get_charset_coll.h>

/** Determines if a MySQL string type is a subset of UTF-8.  This function
 may return false negatives, in case further character-set collation
 codes are introduced in MySQL later.
 @return true if a subset of UTF-8 */
ibool dtype_is_utf8(ulint prtype) /*!< in: precise data type */
{
  /* These codes have been copied from strings/ctype-extra.c
  and strings/ctype-utf8.c. */
  switch (dtype_get_charset_coll(prtype)) {
    case 11:  /* ascii_general_ci */
    case 65:  /* ascii_bin */
    case 33:  /* utf8_general_ci */
    case 83:  /* utf8_bin */
    case 254: /* utf8_general_cs */
      return (TRUE);
  }

  return (FALSE);
}

#endif
