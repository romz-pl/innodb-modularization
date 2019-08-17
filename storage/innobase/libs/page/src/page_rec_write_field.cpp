#include <innodb/page/page_rec_write_field.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/record/rec_get_nth_field_old.h>
#include <innodb/mtr/mlog_write_ulint.h>
#include <innodb/assert/assert.h>

/** Write a 32-bit field in a data dictionary record. */
void page_rec_write_field(rec_t *rec, /*!< in/out: record to update */
                          ulint i,    /*!< in: index of the field to update */
                          ulint val,  /*!< in: value to write */
                          mtr_t *mtr) /*!< in/out: mini-transaction */
{
  byte *data;
  ulint len;

  data = rec_get_nth_field_old(rec, i, &len);

  ut_ad(len == 4);

  mlog_write_ulint(data, val, MLOG_4BYTES, mtr);
}

#endif /* !UNIV_HOTBACKUP */
