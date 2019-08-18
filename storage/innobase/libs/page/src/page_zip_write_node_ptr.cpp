#include <innodb/page/page_zip_write_node_ptr.h>

#include <innodb/memory_check/memory_check.h>
#include <innodb/mtr/mlog_close.h>
#include <innodb/mtr/mlog_open.h>
#include <innodb/mtr/mlog_write_initial_log_record_fast.h>
#include <innodb/mtr/mtr_t.h>
#include <innodb/page/page_offset.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_dir_start.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_heap_no_new.h>

/** Write the node pointer of a record on a non-leaf compressed page. */
void page_zip_write_node_ptr(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    byte *rec,                /*!< in/out: record */
    ulint size,               /*!< in: data size of rec */
    ulint ptr,                /*!< in: node pointer */
    mtr_t *mtr)               /*!< in: mini-transaction, or NULL */
{
  byte *field;
  byte *storage;
#ifdef UNIV_DEBUG
  page_t *page = page_align(rec);
#endif /* UNIV_DEBUG */

  ut_ad(page_simple_validate_new(page));
  ut_ad(page_zip_simple_validate(page_zip));
  ut_ad(page_zip_get_size(page_zip) > PAGE_DATA + page_zip_dir_size(page_zip));
  ut_ad(page_rec_is_comp(rec));

  ut_ad(page_zip->m_start >= PAGE_DATA);
  ut_ad(page_zip_header_cmp(page_zip, page));

  ut_ad(!page_is_leaf(page));

  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));
  UNIV_MEM_ASSERT_RW(rec, size);

  storage = page_zip_dir_start(page_zip) -
            (rec_get_heap_no_new(rec) - 1) * REC_NODE_PTR_SIZE;
  field = rec + size - REC_NODE_PTR_SIZE;

#if defined UNIV_DEBUG || defined UNIV_ZIP_DEBUG
  ut_a(!memcmp(storage, field, REC_NODE_PTR_SIZE));
#endif /* UNIV_DEBUG || UNIV_ZIP_DEBUG */
#if REC_NODE_PTR_SIZE != 4
#error "REC_NODE_PTR_SIZE != 4"
#endif
  mach_write_to_4(field, ptr);
  memcpy(storage, field, REC_NODE_PTR_SIZE);

  if (mtr) {
#ifndef UNIV_HOTBACKUP
    byte *log_ptr = mlog_open(mtr, 11 + 2 + 2 + REC_NODE_PTR_SIZE);
    if (UNIV_UNLIKELY(!log_ptr)) {
      return;
    }

    log_ptr = mlog_write_initial_log_record_fast(field, MLOG_ZIP_WRITE_NODE_PTR,
                                                 log_ptr, mtr);
    mach_write_to_2(log_ptr, page_offset(field));
    log_ptr += 2;
    mach_write_to_2(log_ptr, storage - page_zip->data);
    log_ptr += 2;
    memcpy(log_ptr, field, REC_NODE_PTR_SIZE);
    log_ptr += REC_NODE_PTR_SIZE;
    mlog_close(mtr, log_ptr);
#endif /* !UNIV_HOTBACKUP */
  }
}
