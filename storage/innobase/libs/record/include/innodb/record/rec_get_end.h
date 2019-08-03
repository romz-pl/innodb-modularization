#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>


#ifdef UNIV_DEBUG

/** Returns a pointer to the end of the record.
 @return pointer to end */
UNIV_INLINE
byte *rec_get_end(
    const rec_t *rec,     /*!< in: pointer to record */
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  ut_ad(rec_offs_validate(rec, NULL, offsets));
  return (const_cast<rec_t *>(rec + rec_offs_data_size(offsets)));
}

#else /* UNIV_DEBUG */

#include <innodb/record/rec_offs_data_size.h>

#define rec_get_end(rec, offsets) ((rec) + rec_offs_data_size(offsets))

#endif
