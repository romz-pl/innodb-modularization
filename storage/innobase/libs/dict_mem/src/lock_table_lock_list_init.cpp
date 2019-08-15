#include <innodb/dict_mem/lock_table_lock_list_init.h>

#include <innodb/lock_priv/lock_table_t.h>

/** Initialise the table lock list. */
void lock_table_lock_list_init(
    table_lock_list_t *lock_list) /*!< List to initialise */
{
  UT_LIST_INIT(*lock_list, &lock_table_t::locks);
}
