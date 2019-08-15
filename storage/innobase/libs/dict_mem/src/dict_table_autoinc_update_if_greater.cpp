#include <innodb/dict_mem/dict_table_autoinc_update_if_greater.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_autoinc_own.h>

/** Updates the autoinc counter if the value supplied is greater than the
 current value. */
void dict_table_autoinc_update_if_greater(

    dict_table_t *table, /*!< in/out: table */
    ib_uint64_t value)   /*!< in: value which was assigned to a row */
{
  ut_ad(dict_table_autoinc_own(table));

  if (value > table->autoinc) {
    table->autoinc = value;
  }
}
