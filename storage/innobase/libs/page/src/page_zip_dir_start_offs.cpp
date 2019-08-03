#include <innodb/page/page_zip_dir_start_offs.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/flag.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/assert/assert.h>

/** Gets an offset to the compressed page trailer (the dense page directory),
 including deleted records (the free list).
 @return offset of the dense page directory */
ulint page_zip_dir_start_offs(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    ulint n_dense)                  /*!< in: directory size */
{
  ut_ad(n_dense * PAGE_ZIP_DIR_SLOT_SIZE < page_zip_get_size(page_zip));

  return (page_zip_get_size(page_zip) - n_dense * PAGE_ZIP_DIR_SLOT_SIZE);
}
