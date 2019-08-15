#include <innodb/dict_mem/dict_table_autoinc_initialize.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_autoinc_own.h>

/** Unconditionally set the autoinc counter. */
void dict_table_autoinc_initialize(
    dict_table_t *table, /*!< in/out: table */
    ib_uint64_t value)   /*!< in: next value to assign to a row */
{
  ut_ad(dict_table_autoinc_own(table));

  table->autoinc = value;
}
