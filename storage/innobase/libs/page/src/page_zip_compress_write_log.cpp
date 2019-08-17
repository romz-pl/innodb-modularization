#include <innodb/page/page_zip_compress_write_log.h>

#include <innodb/dict_mem/dict_index_is_ibuf.h>
#include <innodb/disk/page_type_t.h>
#include <innodb/log_types/mlog_id_t.h>
#include <innodb/machine/data.h>
#include <innodb/mtr/mlog_catenate_string.h>
#include <innodb/mtr/mlog_close.h>
#include <innodb/mtr/mlog_open.h>
#include <innodb/mtr/mlog_write_initial_log_record_fast.h>
#include <innodb/page/flag.h>
#include <innodb/page/header.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_is_leaf.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_get_size.h>


#ifndef UNIV_HOTBACKUP
/** Write a log record of compressing an index page. */
void page_zip_compress_write_log(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    const page_t *page,             /*!< in: uncompressed page */
    dict_index_t *index,            /*!< in: index of the B-tree node */
    mtr_t *mtr)                     /*!< in: mini-transaction */
{
  byte *log_ptr;
  ulint trailer_size;

  ut_ad(!dict_index_is_ibuf(index));

  log_ptr = mlog_open(mtr, 11 + 2 + 2);

  if (!log_ptr) {
    return;
  }

  /* Read the number of user records. */
  trailer_size = page_dir_get_n_heap(page_zip->data) - PAGE_HEAP_NO_USER_LOW;
  /* Multiply by uncompressed of size stored per record */
  if (!page_is_leaf(page)) {
    trailer_size *= PAGE_ZIP_DIR_SLOT_SIZE + REC_NODE_PTR_SIZE;
  } else if (index->is_clustered()) {
    trailer_size *=
        PAGE_ZIP_DIR_SLOT_SIZE + DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN;
  } else {
    trailer_size *= PAGE_ZIP_DIR_SLOT_SIZE;
  }
  /* Add the space occupied by BLOB pointers. */
  trailer_size += page_zip->n_blobs * BTR_EXTERN_FIELD_REF_SIZE;
  ut_a(page_zip->m_end > PAGE_DATA);

  static_assert(FIL_PAGE_DATA <= PAGE_DATA, "FIL_PAGE_DATA > PAGE_DATA");

  ut_a(page_zip->m_end + trailer_size <= page_zip_get_size(page_zip));

  log_ptr = mlog_write_initial_log_record_fast(
      (page_t *)page, MLOG_ZIP_PAGE_COMPRESS, log_ptr, mtr);
  mach_write_to_2(log_ptr, page_zip->m_end - FIL_PAGE_TYPE);
  log_ptr += 2;
  mach_write_to_2(log_ptr, trailer_size);
  log_ptr += 2;
  mlog_close(mtr, log_ptr);

  /* Write FIL_PAGE_PREV and FIL_PAGE_NEXT */
  mlog_catenate_string(mtr, page_zip->data + FIL_PAGE_PREV, 4);
  mlog_catenate_string(mtr, page_zip->data + FIL_PAGE_NEXT, 4);
  /* Write most of the page header, the compressed stream and
  the modification log. */
  mlog_catenate_string(mtr, page_zip->data + FIL_PAGE_TYPE,
                       page_zip->m_end - FIL_PAGE_TYPE);
  /* Write the uncompressed trailer of the compressed page. */
  mlog_catenate_string(
      mtr, page_zip->data + page_zip_get_size(page_zip) - trailer_size,
      trailer_size);
}
#endif /* !UNIV_HOTBACKUP */

