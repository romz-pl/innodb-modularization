#include <innodb/page/page_zip_dir_add_slot.h>

#include <innodb/assert/assert.h>
#include <innodb/assert/ASSERT_ZERO.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/flag.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_is_leaf.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/record/flag.h>

#include <string.h>

/** Add a slot to the dense page directory. */
void page_zip_dir_add_slot(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    bool is_clustered)        /*!< in: nonzero for clustered index,
                              zero for others */
{
  ulint n_dense;
  byte *dir;
  byte *stored;

  ut_ad(page_is_comp(page_zip->data));
  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));

  /* Read the old n_dense (n_heap has already been incremented). */
  n_dense = page_dir_get_n_heap(page_zip->data) - (PAGE_HEAP_NO_USER_LOW + 1);

  dir = page_zip->data + page_zip_get_size(page_zip) -
        PAGE_ZIP_DIR_SLOT_SIZE * n_dense;

  if (!page_is_leaf(page_zip->data)) {
    ut_ad(!page_zip->n_blobs);
    stored = dir - n_dense * REC_NODE_PTR_SIZE;
  } else if (is_clustered) {
    /* Move the BLOB pointer array backwards to make space for the
    roll_ptr and trx_id columns and the dense directory slot. */
    byte *externs;

    stored = dir - n_dense * (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);
    externs = stored - page_zip->n_blobs * BTR_EXTERN_FIELD_REF_SIZE;
    ASSERT_ZERO(externs - PAGE_ZIP_CLUST_LEAF_SLOT_SIZE,
                PAGE_ZIP_CLUST_LEAF_SLOT_SIZE);
    memmove(externs - PAGE_ZIP_CLUST_LEAF_SLOT_SIZE, externs, stored - externs);
  } else {
    stored = dir - page_zip->n_blobs * BTR_EXTERN_FIELD_REF_SIZE;
    ASSERT_ZERO(stored - PAGE_ZIP_DIR_SLOT_SIZE,
                static_cast<size_t>(PAGE_ZIP_DIR_SLOT_SIZE));
  }

  /* Move the uncompressed area backwards to make space
  for one directory slot. */
  memmove(stored - PAGE_ZIP_DIR_SLOT_SIZE, stored, dir - stored);
}
