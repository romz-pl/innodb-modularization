#include <innodb/dict_mem/dict_mem_fill_vcol_from_v_indexes.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_index_has_virtual.h>
#include <innodb/dict_mem/dict_mem_fill_vcol_has_index.h>

/** Fill the virtual column set with virtual column of the index
if the index contains the given column name.
@param[in]	col_name	column name
@param[in]	table		innodb table object
@param[out]	v_cols		set of virtual column information. */
void dict_mem_fill_vcol_from_v_indexes(const char *col_name,
                                              const dict_table_t *table,
                                              dict_vcol_set **v_cols) {
  /* virtual column can't be Primary Key, so start with secondary index */
  for (const dict_index_t *index = table->first_index()->next(); index != NULL;
       index = index->next()) {
    /* Skip if the index have newly added
    virtual column because field name is NULL.
    Later virtual column set will be
    refreshed during loading of table. */
    if (!dict_index_has_virtual(index) || index->has_new_v_col) {
      continue;
    }

    for (ulint i = 0; i < index->n_fields; i++) {
      dict_field_t *field = index->get_field(i);

      if (strcmp(field->name, col_name) == 0) {
        dict_mem_fill_vcol_has_index(index, v_cols);
      }
    }
  }
}
