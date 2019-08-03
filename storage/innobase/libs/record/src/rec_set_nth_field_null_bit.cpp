#include <innodb/record/rec_set_nth_field_null_bit.h>

#include <innodb/record/rec_get_1byte_offs_flag.h>
#include <innodb/record/rec_1_get_field_end_info.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_1_set_field_end_info.h>
#include <innodb/record/rec_2_get_field_end_info.h>
#include <innodb/record/rec_2_set_field_end_info.h>

/** Sets the value of the ith field SQL null bit of an old-style record. */
void rec_set_nth_field_null_bit(rec_t *rec, /*!< in: record */
                                ulint i,    /*!< in: ith field */
                                ibool val)  /*!< in: value to set */
{
  ulint info;

  if (rec_get_1byte_offs_flag(rec)) {
    info = rec_1_get_field_end_info(rec, i);

    if (val) {
      info = info | REC_1BYTE_SQL_NULL_MASK;
    } else {
      info = info & ~REC_1BYTE_SQL_NULL_MASK;
    }

    rec_1_set_field_end_info(rec, i, info);

    return;
  }

  info = rec_2_get_field_end_info(rec, i);

  if (val) {
    info = info | REC_2BYTE_SQL_NULL_MASK;
  } else {
    info = info & ~REC_2BYTE_SQL_NULL_MASK;
  }

  rec_2_set_field_end_info(rec, i, info);
}
