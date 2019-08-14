#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

#ifdef UNIV_DEBUG


/** Returns a pointer to the start of the record.
 @return pointer to start */
UNIV_INLINE
byte *rec_get_start(
    const rec_t *rec,     /*!< in: pointer to record */
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  ut_ad(rec_offs_validate(rec, NULL, offsets));
  return (const_cast<rec_t *>(rec - rec_offs_extra_size(offsets)));
}

#else /* UNIV_DEBUG */

#include <innodb/record/rec_offs_extra_size.h>

#define rec_get_start(rec, offsets) ((rec)-rec_offs_extra_size(offsets))

#endif /* UNIV_DEBUG */
