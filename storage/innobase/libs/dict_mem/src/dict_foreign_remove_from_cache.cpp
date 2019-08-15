#include <innodb/dict_mem/dict_foreign_remove_from_cache.h>

#include <innodb/dict_mem/dict_foreign_t.h>
#include <innodb/dict_mem/dict_foreign_free.h>
#include <innodb/dict_mem/dict_table_t.h>

/** Removes a foreign constraint struct from the dictionary cache. */
void dict_foreign_remove_from_cache(
    dict_foreign_t *foreign) /*!< in, own: foreign constraint */
{
  ut_ad(mutex_own(&dict_sys->mutex));
  ut_a(foreign);

  if (foreign->referenced_table != NULL) {
    foreign->referenced_table->referenced_set.erase(foreign);
  }

  if (foreign->foreign_table != NULL) {
    foreign->foreign_table->foreign_set.erase(foreign);
  }

  dict_foreign_free(foreign);
}
