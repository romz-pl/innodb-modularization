#include <innodb/cmp/cmp_dtuple_rec.h>

#include <innodb/cmp/cmp_dtuple_rec_with_match.h>

/** Compare a data tuple to a physical record.
@see cmp_dtuple_rec_with_match
@param[in]	dtuple	data tuple
@param[in]	rec	record
@param[in]	index	index
@param[in]	offsets	rec_get_offsets(rec)
@return the comparison result of dtuple and rec
@retval 0 if dtuple is equal to rec
@retval negative if dtuple is less than rec
@retval positive if dtuple is greater than rec */
int cmp_dtuple_rec(const dtuple_t *dtuple, const rec_t *rec,
                   const dict_index_t *index, const ulint *offsets) {
  ulint matched_fields = 0;

  return (
      cmp_dtuple_rec_with_match(dtuple, rec, index, offsets, &matched_fields));
}
