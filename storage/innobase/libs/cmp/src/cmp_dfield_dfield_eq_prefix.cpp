#include <innodb/cmp/cmp_dfield_dfield_eq_prefix.h>

#include <innodb/data_types/dfield_t.h>
#include <innodb/data_types/dfield_get_type.h>
#include <innodb/data_types/dtype_get_charset_coll.h>
#include <innodb/logger/fatal.h>
#include <innodb/data_types/dfield_get_data.h>
#include <innodb/data_types/dfield_get_len.h>

#include "include/m_ctype.h"

/** Compare two data fields.
@param[in] dfield1 data field
@param[in] dfield2 data field
@return the comparison result of dfield1 and dfield2
@retval true if dfield1 is equal to dfield2, or a prefix of dfield1
@retval false otherwise */
bool cmp_dfield_dfield_eq_prefix(const dfield_t *dfield1,
                                 const dfield_t *dfield2) {
  const dtype_t *type;

  ut_ad(dfield_check_typed(dfield1));
  ut_ad(dfield_check_typed(dfield2));

  type = dfield_get_type(dfield1);

#ifdef UNIV_DEBUG
  switch (type->prtype & DATA_MYSQL_TYPE_MASK) {
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

  uint cs_num = (uint)dtype_get_charset_coll(type->prtype);

  if (CHARSET_INFO *cs = get_charset(cs_num, MYF(MY_WME))) {
    return (!cs->coll->strnncoll(
        cs, static_cast<uchar *>(dfield_get_data(dfield1)),
        dfield_get_len(dfield1), static_cast<uchar *>(dfield_get_data(dfield2)),
        dfield_get_len(dfield2), 1));
  }

#ifdef UNIV_NO_ERR_MSGS
  ib::fatal()
#else
  ib::fatal(ER_IB_MSG_627)
#endif /* !UNIV_NO_ERR_MSGS */
      << "Unable to find charset-collation " << cs_num;

  return (0);
}
