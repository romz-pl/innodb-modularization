#include <innodb/record/rec_fold.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_get_nth_field_instant.h>

/** Compute a hash value of a prefix of a leaf page record.
@param[in]	rec		leaf page record
@param[in]	offsets		rec_get_offsets(rec)
@param[in]	n_fields	number of complete fields to fold
@param[in]	n_bytes		number of bytes to fold in the last field
@param[in]	fold		fold value of the index identifier
@param[in]	index		index where the record resides
@return the folded value */
ulint rec_fold(const rec_t *rec, const ulint *offsets, ulint n_fields,
               ulint n_bytes, ulint fold, const dict_index_t *index) {
  uint16_t i;
  const byte *data;
  ulint len;
  ulint n_fields_rec;

  ut_ad(rec_offs_validate(rec, NULL, offsets));
  ut_ad(rec_validate(rec, offsets));
  ut_ad(n_fields > 0 || n_bytes > 0);

  n_fields_rec = rec_offs_n_fields(offsets);
  ut_ad(n_fields <= n_fields_rec);
  ut_ad(n_fields < n_fields_rec || n_bytes == 0);

  if (n_fields > n_fields_rec) {
    n_fields = n_fields_rec;
  }

  if (n_fields == n_fields_rec) {
    n_bytes = 0;
  }

  for (i = 0; i < n_fields; i++) {
    data = rec_get_nth_field_instant(rec, offsets, i, index, &len);

    if (len != UNIV_SQL_NULL) {
      fold = ut_fold_ulint_pair(fold, ut_fold_binary(data, len));
    }
  }

  if (n_bytes > 0) {
    data = rec_get_nth_field_instant(rec, offsets, i, index, &len);

    if (len != UNIV_SQL_NULL) {
      if (len > n_bytes) {
        len = n_bytes;
      }

      fold = ut_fold_ulint_pair(fold, ut_fold_binary(data, len));
    }
  }

  return (fold);
}



