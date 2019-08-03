#include <innodb/record/rec_get_1byte_offs_flag.h>

#include <innodb/record/rec_get_bit_field_1.h>
#include <innodb/record/flag.h>

/** The following function is used to test whether the data offsets in the
 record are stored in one-byte or two-byte format.
 @return true if 1-byte form */
MY_ATTRIBUTE((warn_unused_result)) ibool
    rec_get_1byte_offs_flag(const rec_t *rec) /*!< in: physical record */
{
#if TRUE != 1
#error "TRUE != 1"
#endif

  return (rec_get_bit_field_1(rec, REC_OLD_SHORT, REC_OLD_SHORT_MASK,
                              REC_OLD_SHORT_SHIFT));
}
