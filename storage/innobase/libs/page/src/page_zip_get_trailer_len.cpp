#include <innodb/page/page_zip_get_trailer_len.h>

#include <innodb/assert/assert.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/flag.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_is_leaf.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_simple_validate.h>
#include <innodb/record/flag.h>

/** Determine if the length of the page trailer.
 @return length of the page trailer, in bytes, not including the
 terminating zero byte of the modification log */
ibool page_zip_get_trailer_len(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    bool is_clust)                  /*!< in: TRUE if clustered index */
{
  ulint uncompressed_size;

  ut_ad(page_zip_simple_validate(page_zip));
  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));

  if (!page_is_leaf(page_zip->data)) {
    uncompressed_size = PAGE_ZIP_DIR_SLOT_SIZE + REC_NODE_PTR_SIZE;
    ut_ad(!page_zip->n_blobs);
  } else if (is_clust) {
    uncompressed_size =
        PAGE_ZIP_DIR_SLOT_SIZE + DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN;
  } else {
    uncompressed_size = PAGE_ZIP_DIR_SLOT_SIZE;
    ut_ad(!page_zip->n_blobs);
  }

  return ((page_dir_get_n_heap(page_zip->data) - 2) * uncompressed_size +
          page_zip->n_blobs * BTR_EXTERN_FIELD_REF_SIZE);
}
