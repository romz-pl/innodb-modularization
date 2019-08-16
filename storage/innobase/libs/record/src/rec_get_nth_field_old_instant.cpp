#include <innodb/record/rec_get_nth_field_old_instant.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/record/rec_get_nth_field_old.h>

/** Gets the value of the specified field in the record in old style.
This is only used for record from instant index, which is clustered
index and has some instantly added columns.
@param[in]	rec	physical record
@param[in]	n	index of the field
@param[in]	index   clustered index where the record resides
@param[out]	len	length of the field, UNIV_SQL if SQL null
@return value of the field, could be either pointer to rec or default value */
const byte *rec_get_nth_field_old_instant(const rec_t *rec, uint16_t n,
                                          const dict_index_t *index,
                                          ulint *len) {
  ut_a(index != nullptr);

  if (n < rec_get_n_fields_old_raw(rec)) {
    return (rec_get_nth_field_old(rec, n, len));
  }

  const byte *field;

  ut_ad(index->has_instant_cols());

  field = index->get_nth_default(n, len);
  return (field);
}
