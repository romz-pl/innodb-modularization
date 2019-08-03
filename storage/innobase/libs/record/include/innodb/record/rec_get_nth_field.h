#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>
#include <innodb/record/rec_get_nth_field_offs.h>

#ifdef UNIV_DEBUG
/** Gets the value of the specified field in the record.
This is used for normal cases, i.e. secondary index or clustered index
which must have no instantly added columns. Also note, if it's non-leaf
page records, it's OK to always use this functioni.
@param[in]	rec	physical record
@param[in]	offsets	array returned by rec_get_offsets()
@param[in]	n	index of the field
@param[out]	len	length of the field, UNIV_SQL_NULL if SQL null
@return value of the field */
inline byte *rec_get_nth_field(const rec_t *rec, const ulint *offsets, ulint n,
                               ulint *len) {
  ulint off = rec_get_nth_field_offs(offsets, n, len);
  ut_ad(*len != UNIV_SQL_ADD_COL_DEFAULT);
  return (const_cast<byte *>(rec) + off);
}
#else /* UNIV_DEBUG */
/** Gets the value of the specified field in the record.
This is used for normal cases, i.e. secondary index or clustered index
which must have no instantly added columns. Also note, if it's non-leaf
page records, it's OK to always use this functioni. */
#define rec_get_nth_field(rec, offsets, n, len) \
  ((rec) + rec_get_nth_field_offs(offsets, n, len))
#endif /* UNIV_DEBUG */
