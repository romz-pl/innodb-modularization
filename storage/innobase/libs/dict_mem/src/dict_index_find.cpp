#include <innodb/dict_mem/dict_index_find.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_table_find_index_on_id.h>
#include <innodb/dict_types/index_id_t.h>
#include <innodb/dict_mem/dict_sys.h>

/** Look up an index.
@param[in]	id	index identifier
@return index or NULL if not found */
const dict_index_t *dict_index_find(const index_id_t &id) {
  const dict_table_t *table;

  ut_ad(mutex_own(&dict_sys->mutex));

  for (table = UT_LIST_GET_FIRST(dict_sys->table_LRU); table != NULL;
       table = UT_LIST_GET_NEXT(table_LRU, table)) {
    const dict_index_t *index = dict_table_find_index_on_id(table, id);
    if (index != NULL) {
      return (index);
    }
  }

  for (table = UT_LIST_GET_FIRST(dict_sys->table_non_LRU); table != NULL;
       table = UT_LIST_GET_NEXT(table_LRU, table)) {
    const dict_index_t *index = dict_table_find_index_on_id(table, id);
    if (index != NULL) {
      return (index);
    }
  }

  return (NULL);
}
