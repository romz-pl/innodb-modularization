#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

#ifdef UNIV_DEBUG

/** Updates debug data in offsets, in order to avoid bogus
 rec_offs_validate() failures. */
UNIV_INLINE
void rec_offs_make_valid(
    const rec_t *rec,          /*!< in: record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint *offsets)            /*!< in: array returned by
                               rec_get_offsets() */
{
  ut_ad(rec);
  ut_ad(index);
  ut_ad(offsets);
  ut_ad(rec_get_n_fields(rec, index) >= rec_offs_n_fields(offsets));
  offsets[2] = (ulint)rec;
  offsets[3] = (ulint)index;
}

#else

#define rec_offs_make_valid(rec, index, offsets) ((void)0)

#endif /* UNIV_DEBUG */


