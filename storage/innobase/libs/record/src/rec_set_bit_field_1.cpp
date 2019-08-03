#include <innodb/record/rec_set_bit_field_1.h>

#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>

/** Sets a bit field within 1 byte. */
void rec_set_bit_field_1(
    rec_t *rec,  /*!< in: pointer to record origin */
    ulint val,   /*!< in: value to set */
    ulint offs,  /*!< in: offset from the origin down */
    ulint mask,  /*!< in: mask used to filter bits */
    ulint shift) /*!< in: shift right applied after masking */
{
  ut_ad(rec);
  ut_ad(offs <= REC_N_OLD_EXTRA_BYTES);
  ut_ad(mask);
  ut_ad(mask <= 0xFFUL);
  ut_ad(((mask >> shift) << shift) == mask);
  ut_ad(((val << shift) & mask) == (val << shift));

  mach_write_to_1(rec - offs,
                  (mach_read_from_1(rec - offs) & ~mask) | (val << shift));
}
