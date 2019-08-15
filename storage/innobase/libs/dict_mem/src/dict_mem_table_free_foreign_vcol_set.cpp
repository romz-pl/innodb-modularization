#include <innodb/dict_mem/dict_mem_table_free_foreign_vcol_set.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_foreign_set.h>
#include <innodb/dict_mem/dict_foreign_t.h>

/** Free the vcol_set from all foreign key constraint on the table.
@param[in,out]	table	innodb table object. */
void dict_mem_table_free_foreign_vcol_set(dict_table_t *table) {
  dict_foreign_set fk_set = table->foreign_set;
  dict_foreign_t *foreign;

  for (auto it = fk_set.begin(); it != fk_set.end(); ++it) {
    foreign = *it;

    if (foreign->v_cols != NULL) {
      UT_DELETE(foreign->v_cols);
      foreign->v_cols = NULL;
    }
  }
}
