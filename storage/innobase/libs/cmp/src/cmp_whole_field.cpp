#include <innodb/cmp/cmp_whole_field.h>

#include <innodb/cmp/cmp_decimal.h>
#include <innodb/cmp/cmp_geometry_field.h>
#include <innodb/cmp/innobase_mysql_cmp.h>
#include <innodb/data_types/flags.h>
#include <innodb/logger/error.h>
#include <innodb/logger/fatal.h>
#include <innodb/machine/data.h>

#include "include/m_ctype.h"

/** Compare two data fields.
@param[in]	mtype		main type
@param[in]	prtype		precise type
@param[in]	is_asc		true=ascending, false=descending order
@param[in]	a		data field
@param[in]	a_length	length of a, in bytes (not UNIV_SQL_NULL)
@param[in]	b		data field
@param[in]	b_length	length of b, in bytes (not UNIV_SQL_NULL)
@return positive, 0, negative, if a is greater, equal, less than b,
respectively */
int cmp_whole_field(ulint mtype, ulint prtype, bool is_asc,
                           const byte *a, unsigned int a_length, const byte *b,
                           unsigned int b_length) {
  float f_1;
  float f_2;
  double d_1;
  double d_2;
  int cmp;

  switch (mtype) {
    case DATA_DECIMAL:
      return (cmp_decimal(a, a_length, b, b_length, is_asc));
    case DATA_DOUBLE:
      d_1 = mach_double_read(a);
      d_2 = mach_double_read(b);

      if (d_1 > d_2) {
        return (is_asc ? 1 : -1);
      } else if (d_2 > d_1) {
        return (is_asc ? -1 : 1);
      }

      return (0);

    case DATA_FLOAT:
      f_1 = mach_float_read(a);
      f_2 = mach_float_read(b);

      if (f_1 > f_2) {
        return (is_asc ? 1 : -1);
      } else if (f_2 > f_1) {
        return (is_asc ? -1 : 1);
      }

      return (0);
    case DATA_VARCHAR:
    case DATA_CHAR:
      cmp = my_charset_latin1.coll->strnncollsp(&my_charset_latin1, a, a_length,
                                                b, b_length);
      break;
    case DATA_BLOB:
      if (prtype & DATA_BINARY_TYPE) {
        ib::error(ER_IB_MSG_920) << "Comparing a binary BLOB"
                                    " using a character set collation!";
        ut_ad(0);
      }
      /* fall through */
    case DATA_VARMYSQL:
    case DATA_MYSQL:
      cmp = innobase_mysql_cmp(prtype, a, a_length, b, b_length);
      break;
    case DATA_POINT:
    case DATA_VAR_POINT:
    case DATA_GEOMETRY:
      return (cmp_geometry_field(mtype, prtype, a, a_length, b, b_length));
    default:
      ib::fatal(ER_IB_MSG_921) << "Unknown data type number " << mtype;
      cmp = 0;
  }
  if (!is_asc) {
    cmp = -cmp;
  }
  return (cmp);
}
