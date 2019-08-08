#include <innodb/tablespace/xdes_get_bit.h>

#include <innodb/bit/ut_bit_get_nth.h>
#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>
#include <innodb/tablespace/consts.h>
#include <innodb/log_types/mlog_id_t.h>
#include <innodb/tablespace/mach_read_ulint.h>


/** Gets a descriptor bit of a page.
 @return true if free */
ibool xdes_get_bit(const xdes_t *descr, /*!< in: descriptor */
                   ulint bit,        /*!< in: XDES_FREE_BIT or XDES_CLEAN_BIT */
                   page_no_t offset) /*!< in: page offset within extent:
                                     0 ... FSP_EXTENT_SIZE - 1 */
{
  ut_ad(offset < FSP_EXTENT_SIZE);
  ut_ad(bit == XDES_FREE_BIT || bit == XDES_CLEAN_BIT);

  ulint index = bit + XDES_BITS_PER_PAGE * offset;

  ulint bit_index = index % 8;
  ulint byte_index = index / 8;

  return (ut_bit_get_nth(
      mach_read_ulint(descr + XDES_BITMAP + byte_index, MLOG_1BYTE),
      bit_index));
}

