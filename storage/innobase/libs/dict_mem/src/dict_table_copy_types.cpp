#include <innodb/dict_mem/dict_table_copy_types.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_copy_v_types.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dtuple_get_n_fields.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/dfield_set_null.h>
#include <innodb/data_types/dfield_get_type.h>

/** Copies types of columns contained in table to tuple and sets all
 fields of the tuple to the SQL NULL value.  This function should
 be called right after dtuple_create(). */
void dict_table_copy_types(dtuple_t *tuple,           /*!< in/out: data tuple */
                           const dict_table_t *table) /*!< in: table */
{
  ulint i;

  for (i = 0; i < dtuple_get_n_fields(tuple); i++) {
    dfield_t *dfield = dtuple_get_nth_field(tuple, i);
    dtype_t *dtype = dfield_get_type(dfield);

    dfield_set_null(dfield);
    table->get_col(i)->copy_type(dtype);
  }

  dict_table_copy_v_types(tuple, table);
}
