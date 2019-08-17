#include <innodb/page/page_dir_set_n_heap.h>

#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_header_get_field.h>
#include <innodb/page/page_header_set_field.h>
#include <innodb/assert/assert.h>
#include <innodb/page/header.h>

/** Sets the number of records in the heap. */
void page_dir_set_n_heap(
    page_t *page,             /*!< in/out: index page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page whose
                             uncompressed part will be updated, or NULL.
                             Note that the size of the dense page directory
                             in the compressed page trailer is
                             n_heap * PAGE_ZIP_DIR_SLOT_SIZE. */
    ulint n_heap)             /*!< in: number of records */
{
  ut_ad(n_heap < 0x8000);
  ut_ad(!page_zip ||
        n_heap == (page_header_get_field(page, PAGE_N_HEAP) & 0x7fff) + 1);

  page_header_set_field(
      page, page_zip, PAGE_N_HEAP,
      n_heap | (0x8000 & page_header_get_field(page, PAGE_N_HEAP)));
}
