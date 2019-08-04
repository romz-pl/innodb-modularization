#include <innodb/data_types/innobase_get_cset_width.h>

#include <innodb/assert/assert.h>
#include <innodb/data_types/flags.h>

#include "m_ctype.h"
#include "sql/sql_class.h"
#include "sql/log.h"
#include "mysql/plugin.h"

/** Get the variable length bounds of the given character set. */
void innobase_get_cset_width(
    ulint cset,      /*!< in: MySQL charset-collation code */
    ulint *mbminlen, /*!< out: minimum length of a char (in bytes) */
    ulint *mbmaxlen) /*!< out: maximum length of a char (in bytes) */
{
  CHARSET_INFO *cs;
  ut_ad(cset <= MAX_CHAR_COLL_NUM);
  ut_ad(mbminlen);
  ut_ad(mbmaxlen);

  cs = all_charsets[cset];
  if (cs) {
    *mbminlen = cs->mbminlen;
    *mbmaxlen = cs->mbmaxlen;
    ut_ad(*mbminlen < DATA_MBMAX);
    ut_ad(*mbmaxlen < DATA_MBMAX);
  } else {
    THD *thd = current_thd;

    if (thd && thd_sql_command(thd) == SQLCOM_DROP_TABLE) {
      /* Fix bug#46256: allow tables to be dropped if the
      collation is not found, but issue a warning. */
      if (cset != 0) {
        log_errlog(ERROR_LEVEL, ER_INNODB_UNKNOWN_COLLATION);
      }
    } else {
      ut_a(cset == 0);
    }

    *mbminlen = *mbmaxlen = 0;
  }
}
