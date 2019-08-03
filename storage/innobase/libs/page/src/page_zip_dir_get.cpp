#include <innodb/page/page_zip_dir_get.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_simple_validate.h>
#include <innodb/page/page_zip_dir_size.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/flag.h>
#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>

/** Read a given slot in the dense page directory.
 @return record offset on the uncompressed page, possibly ORed with
 PAGE_ZIP_DIR_SLOT_DEL or PAGE_ZIP_DIR_SLOT_OWNED */
ulint page_zip_dir_get(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    ulint slot)                     /*!< in: slot
                                    (0=first user record) */
{
  ut_ad(page_zip_simple_validate(page_zip));
  ut_ad(slot < page_zip_dir_size(page_zip) / PAGE_ZIP_DIR_SLOT_SIZE);
  return (mach_read_from_2(page_zip->data + page_zip_get_size(page_zip) -
                           PAGE_ZIP_DIR_SLOT_SIZE * (slot + 1)));
}
