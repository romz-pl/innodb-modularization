#include <innodb/record/rec_set_bit_field_2.h>

#include <innodb/record/flag.h>
#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>

/** Sets a bit field within 2 bytes. */
void rec_set_bit_field_2(
    rec_t *rec,  /*!< in: pointer to record origin */
    ulint val,   /*!< in: value to set */
    ulint offs,  /*!< in: offset from the origin down */
    ulint mask,  /*!< in: mask used to filter bits */
    ulint shift) /*!< in: shift right applied after masking */
{
  ut_ad(rec);
  ut_ad(offs <= REC_N_OLD_EXTRA_BYTES);
  ut_ad(mask > 0xFFUL);
  ut_ad(mask <= 0xFFFFUL);
  ut_ad((mask >> shift) & 1);
  ut_ad(0 == ((mask >> shift) & ((mask >> shift) + 1)));
  ut_ad(((mask >> shift) << shift) == mask);
  ut_ad(((val << shift) & mask) == (val << shift));

  mach_write_to_2(rec - offs,
                  (mach_read_from_2(rec - offs) & ~mask) | (val << shift));
}
