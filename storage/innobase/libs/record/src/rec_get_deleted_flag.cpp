#include <innodb/record/rec_get_deleted_flag.h>

#include <innodb/record/rec_get_bit_field_1.h>
#include <innodb/record/flag.h>

/** The following function tells if record is delete marked.
 @return nonzero if delete marked */
ulint rec_get_deleted_flag(const rec_t *rec, /*!< in: physical record */
                           ulint comp) /*!< in: nonzero=compact page format */
{
  if (comp) {
    return (rec_get_bit_field_1(rec, REC_NEW_INFO_BITS, REC_INFO_DELETED_FLAG,
                                REC_INFO_BITS_SHIFT));
  } else {
    return (rec_get_bit_field_1(rec, REC_OLD_INFO_BITS, REC_INFO_DELETED_FLAG,
                                REC_INFO_BITS_SHIFT));
  }
}
