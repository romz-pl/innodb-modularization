#include <innodb/dict_mem/dict_table_add_to_cache.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/hash/HASH_SEARCH.h>
#include <innodb/hash/HASH_INSERT.h>
#include <innodb/random/random.h>
#include <innodb/dict_mem/dict_table_set_big_rows.h>
#include <innodb/memory/mem_heap_get_size.h>
#include <innodb/dict_mem/dict_sys.h>

/** Adds a table object to the dictionary cache.
@param[in,out]	table		table
@param[in]	can_be_evicted	true if can be evicted
@param[in,out]	heap		temporary heap
*/
void dict_table_add_to_cache(dict_table_t *table, ibool can_be_evicted,
                             mem_heap_t *heap) {
  ulint fold;
  ulint id_fold;

  ut_ad(dict_lru_validate());
  ut_ad(mutex_own(&dict_sys->mutex));

  table->cached = true;

  fold = ut_fold_string(table->name.m_name);
  id_fold = ut_fold_ull(table->id);

  dict_table_set_big_rows(table);

  /* Look for a table with the same name: error if such exists */
  {
    dict_table_t *table2;
    HASH_SEARCH(name_hash, dict_sys->table_hash, fold, dict_table_t *, table2,
                ut_ad(table2->cached),
                !strcmp(table2->name.m_name, table->name.m_name));
    ut_a(table2 == NULL);

#ifdef UNIV_DEBUG
    /* Look for the same table pointer with a different name */
    HASH_SEARCH_ALL(name_hash, dict_sys->table_hash, dict_table_t *, table2,
                    ut_ad(table2->cached), table2 == table);
    ut_ad(table2 == NULL);
#endif /* UNIV_DEBUG */
  }

  /* Look for a table with the same id: error if such exists */
  {
    dict_table_t *table2;
    HASH_SEARCH(id_hash, dict_sys->table_id_hash, id_fold, dict_table_t *,
                table2, ut_ad(table2->cached), table2->id == table->id);
    ut_a(table2 == NULL);

#ifdef UNIV_DEBUG
    /* Look for the same table pointer with a different id */
    HASH_SEARCH_ALL(id_hash, dict_sys->table_id_hash, dict_table_t *, table2,
                    ut_ad(table2->cached), table2 == table);
    ut_ad(table2 == NULL);
#endif /* UNIV_DEBUG */
  }

  /* Add table to hash table of tables */
  HASH_INSERT(dict_table_t, name_hash, dict_sys->table_hash, fold, table);

  /* Add table to hash table of tables based on table id */
  HASH_INSERT(dict_table_t, id_hash, dict_sys->table_id_hash, id_fold, table);

  table->can_be_evicted = can_be_evicted;

  if (table->can_be_evicted) {
    UT_LIST_ADD_FIRST(dict_sys->table_LRU, table);
  } else {
    UT_LIST_ADD_FIRST(dict_sys->table_non_LRU, table);
  }

  ut_ad(dict_lru_validate());

  table->dirty_status.store(METADATA_CLEAN);

  dict_sys->size +=
      mem_heap_get_size(table->heap) + strlen(table->name.m_name) + 1;
  DBUG_EXECUTE_IF(
      "dd_upgrade", if (srv_is_upgrade_mode && srv_upgrade_old_undo_found) {
        ib::info(ER_IB_MSG_176) << "Adding table to cache: " << table->name;
      });
}
