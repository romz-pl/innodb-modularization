#include <innodb/page/page_zip_write_blob_ptr.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/mtr/mlog_close.h>
#include <innodb/mtr/mlog_open.h>
#include <innodb/mtr/mlog_write_initial_log_record_fast.h>
#include <innodb/mtr/mtr_t.h>
#include <innodb/page/flag.h>
#include <innodb/page/page_align.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_offset.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_get_n_prev_extern.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/record/rec_get_n_extern_new.h>
#include <innodb/record/rec_get_nth_field.h>


/** Write a BLOB pointer of a record on the leaf page of a clustered index.
 The information must already have been updated on the uncompressed page. */
void page_zip_write_blob_ptr(
    page_zip_des_t *page_zip,  /*!< in/out: compressed page */
    const byte *rec,           /*!< in/out: record whose data is being
                               written */
    const dict_index_t *index, /*!< in: index of the page */
    const ulint *offsets,      /*!< in: rec_get_offsets(rec, index) */
    ulint n,                   /*!< in: column index */
    mtr_t *mtr)                /*!< in: mini-transaction handle,
                       or NULL if no logging is needed */
{
  const byte *field;
  byte *externs;
  const page_t *page = page_align(rec);
  ulint blob_no;
  ulint len;

  ut_ad(page_zip != NULL);
  ut_ad(rec != NULL);
  ut_ad(index != NULL);
  ut_ad(offsets != NULL);
  ut_ad(page_simple_validate_new((page_t *)page));
  ut_ad(page_zip_simple_validate(page_zip));
  ut_ad(page_zip_get_size(page_zip) > PAGE_DATA + page_zip_dir_size(page_zip));
  ut_ad(rec_offs_comp(offsets));
  ut_ad(rec_offs_validate(rec, NULL, offsets));
  ut_ad(rec_offs_any_extern(offsets));
  ut_ad(rec_offs_nth_extern(offsets, n));

  ut_ad(page_zip->m_start >= PAGE_DATA);
  ut_ad(page_zip_header_cmp(page_zip, page));

  ut_ad(page_is_leaf(page));
  ut_ad(index->is_clustered());

  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));
  UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
  UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                     rec_offs_extra_size(offsets));

  blob_no = page_zip_get_n_prev_extern(page_zip, rec, index) +
            rec_get_n_extern_new(rec, index, n);
  ut_a(blob_no < page_zip->n_blobs);

  externs = page_zip->data + page_zip_get_size(page_zip) -
            (page_dir_get_n_heap(page) - PAGE_HEAP_NO_USER_LOW) *
                PAGE_ZIP_CLUST_LEAF_SLOT_SIZE;

  field = rec_get_nth_field(rec, offsets, n, &len);

  externs -= (blob_no + 1) * BTR_EXTERN_FIELD_REF_SIZE;
  field += len - BTR_EXTERN_FIELD_REF_SIZE;

  memcpy(externs, field, BTR_EXTERN_FIELD_REF_SIZE);

#ifdef UNIV_ZIP_DEBUG
  ut_a(page_zip_validate(page_zip, page, index));
#endif /* UNIV_ZIP_DEBUG */

  if (mtr) {
#ifndef UNIV_HOTBACKUP
    byte *log_ptr = mlog_open(mtr, 11 + 2 + 2 + BTR_EXTERN_FIELD_REF_SIZE);
    if (UNIV_UNLIKELY(!log_ptr)) {
      return;
    }

    log_ptr = mlog_write_initial_log_record_fast(
        (byte *)field, MLOG_ZIP_WRITE_BLOB_PTR, log_ptr, mtr);
    mach_write_to_2(log_ptr, page_offset(field));
    log_ptr += 2;
    mach_write_to_2(log_ptr, externs - page_zip->data);
    log_ptr += 2;
    memcpy(log_ptr, externs, BTR_EXTERN_FIELD_REF_SIZE);
    log_ptr += BTR_EXTERN_FIELD_REF_SIZE;
    mlog_close(mtr, log_ptr);
#endif /* !UNIV_HOTBACKUP */
  }
}
