#include <innodb/dict_mem/dict_table_find_index_on_id.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_types/index_id_t.h>

/** Look up an index in a table.
@param[in]	table	table
@param[in]	id	index identifier
@return index
@retval NULL if not found */
const dict_index_t *dict_table_find_index_on_id(
    const dict_table_t *table, const index_id_t &id) {
  for (const dict_index_t *index = table->first_index(); index != NULL;
       index = index->next()) {
    if (index->space == id.m_space_id && index->id == id.m_index_id) {
      return (index);
    }
  }

  return (NULL);
}
