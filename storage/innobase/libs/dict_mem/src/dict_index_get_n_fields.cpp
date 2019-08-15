#include <innodb/dict_mem/dict_index_get_n_fields.h>

#include <innodb/dict_mem/dict_index_t.h>

/** Gets the number of fields in the internal representation of an index,
 including fields added by the dictionary system.
 @return number of fields */
ulint dict_index_get_n_fields(
    const dict_index_t *index) /*!< in: an internal
                               representation of index (in
                               the dictionary cache) */
{
  ut_ad(index);
  ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

  return index->n_fields;
}
