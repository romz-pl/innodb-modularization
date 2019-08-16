#include <innodb/record/rec_get_nth_field_instant.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/record/rec_get_nth_field_offs.h>

/** Gets the value of the specified field in the record.
This is only used when there is possibility that the record comes from the
clustered index, which has some instantly add columns
@param[in]	rec	record
@param[in]	offsets	array returned by rec_get_offsets()
@param[in]	n	index of the field
@param[in]	index	clustered index where the record resides
@param[in,out]	len	length of the field, UNIV_SQL_NULL if SQL null
@return	value of the field, could be either pointer to rec or default value */
const byte *rec_get_nth_field_instant(const rec_t *rec, const ulint *offsets,
                                      ulint n, const dict_index_t *index,
                                      ulint *len) {
  ulint off = rec_get_nth_field_offs(offsets, n, len);

  if (*len != UNIV_SQL_ADD_COL_DEFAULT) {
    return (rec + off);
  }

  ut_a(index != nullptr);
  ut_ad(index->has_instant_cols());

  return (index->get_nth_default(n, len));
}
