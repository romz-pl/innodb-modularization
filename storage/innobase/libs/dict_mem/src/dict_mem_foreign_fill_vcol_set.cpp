#include <innodb/dict_mem/dict_mem_foreign_fill_vcol_set.h>

#include <innodb/dict_mem/dict_foreign_t.h>
#include <innodb/dict_mem/dict_mem_fill_vcol_set_for_base_col.h>
#include <innodb/dict_mem/dict_mem_fill_vcol_from_v_indexes.h>

/** Fills the dependent virtual columns in a set.
Reason for being dependent are
1) FK can be present on base column of virtual columns
2) FK can be present on column which is a part of virtual index
@param[in,out]  foreign foreign key information. */
void dict_mem_foreign_fill_vcol_set(dict_foreign_t *foreign) {
  ulint type = foreign->type;

  if (type == 0) {
    return;
  }

  for (ulint i = 0; i < foreign->n_fields; i++) {
    /** FK can be present on base columns
    of virtual columns. */
    dict_mem_fill_vcol_set_for_base_col(foreign->foreign_col_names[i],
                                        foreign->foreign_table,
                                        &foreign->v_cols);

    /** FK can be present on the columns
    which can be a part of virtual index. */
    dict_mem_fill_vcol_from_v_indexes(foreign->foreign_col_names[i],
                                      foreign->foreign_table, &foreign->v_cols);
  }
}
