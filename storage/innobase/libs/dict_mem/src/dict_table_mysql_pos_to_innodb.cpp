#include <innodb/dict_mem/dict_table_mysql_pos_to_innodb.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_v_col_t.h>

/** Get the innodb column position for a non-virtual column according to
its original MySQL table position n
@param[in]	table	table
@param[in]	n	MySQL column position
@return column position in InnoDB */
ulint dict_table_mysql_pos_to_innodb(const dict_table_t *table, ulint n) {
  ut_ad(n < table->n_t_cols);

  if (table->n_v_def == 0) {
    /* No virtual columns, the MySQL position is the same
    as InnoDB position */
    return (n);
  }

  /* Find out how many virtual columns are stored in front of 'n' */
  ulint v_before = 0;
  for (ulint i = 0; i < table->n_v_def; ++i) {
    if (table->v_cols[i].m_col.ind > n) {
      break;
    }

    ++v_before;
  }

  ut_ad(n >= v_before);

  return (n - v_before);
}
