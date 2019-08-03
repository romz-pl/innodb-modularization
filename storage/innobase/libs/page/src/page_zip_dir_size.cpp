#include <innodb/page/page_zip_dir_size.h>

#include <innodb/page/flag.h>
#include <innodb/page/page_zip_dir_elems.h>

/** Gets the size of the compressed page trailer (the dense page directory),
 including deleted records (the free list).
 @return length of dense page directory, in bytes */
ulint page_zip_dir_size(
    const page_zip_des_t *page_zip) /*!< in: compressed page */
{
  return (PAGE_ZIP_DIR_SLOT_SIZE * page_zip_dir_elems(page_zip));
}
