#include <innodb/dict_mem/dict_table_get_all_fts_indexes.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/vector/vector.h>

/** Get all the FTS indexes on a table.
@param[in]	table	table
@param[out]	indexes	all FTS indexes on this table
@return number of FTS indexes */
ulint dict_table_get_all_fts_indexes(dict_table_t *table,
                                     ib_vector_t *indexes) {
  dict_index_t *index;

  ut_a(ib_vector_size(indexes) == 0);

  for (index = table->first_index(); index; index = index->next()) {
    if (index->type == DICT_FTS) {
      ib_vector_push(indexes, &index);
    }
  }

  return (ib_vector_size(indexes));
}
