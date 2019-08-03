#include <innodb/page/page_zip_dir_user_size.h>

#include <innodb/assert/assert.h>
#include <innodb/page/page_get_n_recs.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_dir_size.h>
#include <innodb/page/flag.h>

/** Gets the size of the compressed page trailer (the dense page directory),
 only including user records (excluding the free list).
 @return length of dense page directory comprising existing records, in bytes */
ulint page_zip_dir_user_size(
    const page_zip_des_t *page_zip) /*!< in: compressed page */
{
  ulint size = PAGE_ZIP_DIR_SLOT_SIZE * page_get_n_recs(page_zip->data);
  ut_ad(size <= page_zip_dir_size(page_zip));
  return (size);
}
