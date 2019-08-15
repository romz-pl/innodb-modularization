#include <innodb/dict_mem/dict_table_copy_v_types.h>

#include <innodb/data_types/dfield_get_type.h>
#include <innodb/data_types/dfield_set_null.h>
#include <innodb/data_types/dtuple_get_n_v_fields.h>
#include <innodb/data_types/dtuple_get_nth_v_field.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/dict_mem/dict_table_get_nth_v_col.h>
#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_v_col_t.h>

#include <utility>

/** Copies types of virtual columns contained in table to tuple and sets all
fields of the tuple to the SQL NULL value.  This function should
be called right after dtuple_create().
@param[in,out]	tuple	data tuple
@param[in]	table	table
*/
void dict_table_copy_v_types(dtuple_t *tuple, const dict_table_t *table) {
  /* tuple could have more virtual columns than existing table,
  if we are calling this for creating index along with adding
  virtual columns */
  ulint n_fields =
      std::min(dtuple_get_n_v_fields(tuple), static_cast<ulint>(table->n_v_def));

  for (ulint i = 0; i < n_fields; i++) {
    dfield_t *dfield = dtuple_get_nth_v_field(tuple, i);
    dtype_t *dtype = dfield_get_type(dfield);

    dfield_set_null(dfield);
    dict_table_get_nth_v_col(table, i)->m_col.copy_type(dtype);
  }
}
