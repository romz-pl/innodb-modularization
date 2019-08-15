#include <innodb/dict_mem/dict_lru_validate.h>


#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG

/** Validate the dictionary table LRU list.
 @return true if valid */
static ibool dict_lru_validate(void) {
  dict_table_t *table;

  ut_ad(mutex_own(&dict_sys->mutex));

  for (table = UT_LIST_GET_FIRST(dict_sys->table_LRU); table != NULL;
       table = UT_LIST_GET_NEXT(table_LRU, table)) {
    ut_a(table->can_be_evicted);
  }

  for (table = UT_LIST_GET_FIRST(dict_sys->table_non_LRU); table != NULL;
       table = UT_LIST_GET_NEXT(table_LRU, table)) {
    ut_a(!table->can_be_evicted);
  }

  return (TRUE);
}


#endif /* UNIV_DEBUG */
#endif
