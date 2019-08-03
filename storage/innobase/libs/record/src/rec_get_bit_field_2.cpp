#include <innodb/record/rec_get_bit_field_2.h>

#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>

/** Gets a bit field from within 2 bytes. */
uint16_t rec_get_bit_field_2(
    const rec_t *rec, /*!< in: pointer to record origin */
    ulint offs,       /*!< in: offset from the origin down */
    ulint mask,       /*!< in: mask used to filter bits */
    ulint shift)      /*!< in: shift right applied after masking */
{
  ut_ad(rec);

  return ((mach_read_from_2(rec - offs) & mask) >> shift);
}
