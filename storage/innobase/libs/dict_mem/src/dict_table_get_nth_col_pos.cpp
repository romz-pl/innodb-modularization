#include <innodb/dict_mem/dict_table_get_nth_col_pos.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Looks for non-virtual column n position in the clustered index.
 @return position in internal representation of the clustered index */
ulint dict_table_get_nth_col_pos(const dict_table_t *table, /*!< in: table */
                                 ulint n) /*!< in: column number */
{
  return (table->first_index()->get_col_pos(n));
}
