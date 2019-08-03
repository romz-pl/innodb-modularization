#include <innodb/page/page_zip_dir_find_low.h>

#include <innodb/machine/data.h>
#include <innodb/assert/assert.h>
#include <innodb/page/flag.h>

/** Find the slot of the given record in the dense page directory.
 @return dense directory slot, or NULL if record not found */
byte *page_zip_dir_find_low(byte *slot,   /*!< in: start of records */
                            byte *end,    /*!< in: end of records */
                            ulint offset) /*!< in: offset of user record */
{
  ut_ad(slot <= end);

  for (; slot < end; slot += PAGE_ZIP_DIR_SLOT_SIZE) {
    if ((mach_read_from_2(slot) & PAGE_ZIP_DIR_SLOT_MASK) == offset) {
      return (slot);
    }
  }

  return (NULL);
}
