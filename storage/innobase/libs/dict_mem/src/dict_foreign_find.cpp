#include <innodb/dict_mem/dict_foreign_find.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_foreign_t.h>
#include <innodb/dict_mem/dict_foreign_set_validate.h>

/** Looks for the foreign constraint from the foreign and referenced lists
 of a table.
 @return foreign constraint */
dict_foreign_t *dict_foreign_find(
    dict_table_t *table,     /*!< in: table object */
    dict_foreign_t *foreign) /*!< in: foreign constraint */
{
  ut_ad(mutex_own(&dict_sys->mutex));

  ut_ad(dict_foreign_set_validate(table->foreign_set));
  ut_ad(dict_foreign_set_validate(table->referenced_set));

  dict_foreign_set::iterator it = table->foreign_set.find(foreign);

  if (it != table->foreign_set.end()) {
    return (*it);
  }

  it = table->referenced_set.find(foreign);

  if (it != table->referenced_set.end()) {
    return (*it);
  }

  return (NULL);
}
