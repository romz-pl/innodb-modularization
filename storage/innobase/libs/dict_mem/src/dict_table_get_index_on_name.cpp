#include <innodb/dict_mem/dict_table_get_index_on_name.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/string/innobase_strcasecmp.h>

/** Get an index by name.
@param[in]	table		the table where to look for the index
@param[in]	name		the index name to look for
@param[in]	committed	true=search for committed,
false=search for uncommitted
@return index, NULL if does not exist */
dict_index_t *dict_table_get_index_on_name(dict_table_t *table,
                                           const char *name, bool committed) {
  dict_index_t *index;

  index = table->first_index();

  while (index != NULL) {
    if (index->is_committed() == committed &&
        innobase_strcasecmp(index->name, name) == 0) {
      return (index);
    }

    index = index->next();
  }

  return (NULL);
}
