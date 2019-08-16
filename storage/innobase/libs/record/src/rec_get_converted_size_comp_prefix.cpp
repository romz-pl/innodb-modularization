#include <innodb/record/rec_get_converted_size_comp_prefix.h>

#include <innodb/record/rec_get_converted_size_comp_prefix_low.h>
#include <innodb/dict_mem/dict_table_is_comp.h>
#include <innodb/assert/assert.h>

/** Determines the size of a data tuple prefix in ROW_FORMAT=COMPACT.
 @return total size */
ulint rec_get_converted_size_comp_prefix(
    const dict_index_t *index, /*!< in: record descriptor */
    const dfield_t *fields,    /*!< in: array of data fields */
    ulint n_fields,            /*!< in: number of data fields */
    ulint *extra)              /*!< out: extra size */
{
  ut_ad(dict_table_is_comp(index->table));
  return (rec_get_converted_size_comp_prefix_low(index, fields, n_fields, NULL,
                                                 extra, nullptr, false));
}

