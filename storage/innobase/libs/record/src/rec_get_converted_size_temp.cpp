#include <innodb/record/rec_get_converted_size_temp.h>

#include <innodb/record/rec_get_converted_size_comp_prefix_low.h>

/** Determines the size of a data tuple prefix in ROW_FORMAT=COMPACT.
 @return total size */
ulint rec_get_converted_size_temp(
    const dict_index_t *index, /*!< in: record descriptor */
    const dfield_t *fields,    /*!< in: array of data fields */
    ulint n_fields,            /*!< in: number of data fields */
    const dtuple_t *v_entry,   /*!< in: dtuple contains virtual column
                               data */
    ulint *extra)              /*!< out: extra size */
{
  return (rec_get_converted_size_comp_prefix_low(
      index, fields, n_fields, v_entry, extra, nullptr, true));
}
