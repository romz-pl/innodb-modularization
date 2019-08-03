#include <innodb/record/rec_offs_set_n_fields.h>

#include <innodb/assert/assert.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_offs_get_n_alloc.h>

/** The following function sets the number of fields in offsets. */
void rec_offs_set_n_fields(ulint *offsets, /*!< in/out: array returned by
                                           rec_get_offsets() */
                           ulint n_fields) /*!< in: number of fields */
{
  ut_ad(offsets);
  ut_ad(n_fields > 0);
  ut_ad(n_fields <= REC_MAX_N_FIELDS);
  ut_ad(n_fields + REC_OFFS_HEADER_SIZE <= rec_offs_get_n_alloc(offsets));
  offsets[1] = n_fields;
}
