#include <innodb/dict_mem/dict_table_autoinc_read.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_autoinc_own.h>

/** Reads the next autoinc value (== autoinc counter value), 0 if not yet
 initialized.
 @return value for a new row, or 0 */
ib_uint64_t dict_table_autoinc_read(const dict_table_t *table) /*!< in: table */
{
  ut_ad(dict_table_autoinc_own(table));

  return (table->autoinc);
}
