#include <innodb/dict_mem/dict_mem_table_fill_foreign_vcol_set.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_foreign_set.h>
#include <innodb/dict_mem/dict_foreign_t.h>
#include <innodb/dict_mem/dict_mem_foreign_fill_vcol_set.h>

/** Fill virtual columns set in each fk constraint present in the table.
@param[in,out]	table	innodb table object. */
void dict_mem_table_fill_foreign_vcol_set(dict_table_t *table) {
  dict_foreign_set fk_set = table->foreign_set;
  dict_foreign_t *foreign;

  for (auto it = fk_set.begin(); it != fk_set.end(); ++it) {
    foreign = *it;

    dict_mem_foreign_fill_vcol_set(foreign);
  }
}
