#include <innodb/cmp/innobase_mysql_cmp.h>

#include <innodb/data_types/dtype_get_charset_coll.h>
#include <innodb/logger/fatal.h>
#include <innodb/data_types/flags.h>

#include "include/my_sys.h"
#include "include/m_ctype.h"
#include "include/field_types.h"

/** Compare two data fields.
@param[in] prtype precise type
@param[in] a data field
@param[in] a_length length of a, in bytes (not UNIV_SQL_NULL)
@param[in] b data field
@param[in] b_length length of b, in bytes (not UNIV_SQL_NULL)
@return positive, 0, negative, if a is greater, equal, less than b,
respectively */
int innobase_mysql_cmp(ulint prtype, const byte *a, size_t a_length,
                       const byte *b, size_t b_length) {
#ifdef UNIV_DEBUG
  switch (prtype & DATA_MYSQL_TYPE_MASK) {
    case MYSQL_TYPE_BIT:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_VARCHAR:
      break;
    default:
      ut_error;
  }
#endif /* UNIV_DEBUG */

  uint cs_num = (uint)dtype_get_charset_coll(prtype);

  if (CHARSET_INFO *cs = get_charset(cs_num, MYF(MY_WME))) {
    if ((prtype & DATA_MYSQL_TYPE_MASK) == MYSQL_TYPE_STRING &&
        cs->pad_attribute == NO_PAD) {
      /* MySQL specifies that CHAR fields are stripped of
      trailing spaces before being returned from the database.
      Normally this is done in Field_string::val_str(),
      but since we don't involve the Field classes for internal
      index comparisons, we need to do the same thing here
      for NO PAD collations. (If not, strnncollsp will ignore
      the spaces for us, so we don't need to do it here.) */
      a_length = cs->cset->lengthsp(cs, (const char *)a, a_length);
      b_length = cs->cset->lengthsp(cs, (const char *)b, b_length);
    }

    return (cs->coll->strnncollsp(cs, a, a_length, b, b_length));
  }

  ib::fatal(ER_IB_MSG_919) << "Unable to find charset-collation " << cs_num;
  return (0);
}
