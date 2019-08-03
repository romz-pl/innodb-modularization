#include <innodb/record/rec_offs_cmp.h>


#ifdef UNIV_DEBUG

/** Check if the given two record offsets are identical.
@param[in]  offsets1  field offsets of a record
@param[in]  offsets2  field offsets of a record
@return true if they are identical, false otherwise. */
bool rec_offs_cmp(ulint *offsets1, ulint *offsets2) {
  ulint n1 = rec_offs_n_fields(offsets1);
  ulint n2 = rec_offs_n_fields(offsets2);

  if (n1 != n2) {
    return (false);
  }

  for (ulint i = 0; i < n1; ++i) {
    ulint len_1;
    ulint field_offset_1 = rec_get_nth_field_offs(offsets1, i, &len_1);

    ulint len_2;
    ulint field_offset_2 = rec_get_nth_field_offs(offsets2, i, &len_2);

    if (field_offset_1 != field_offset_2) {
      return (false);
    }

    if (len_1 != len_2) {
      return (false);
    }
  }
  return (true);
}

#endif
