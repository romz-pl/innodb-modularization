#include <innodb/page/page_zip_copy_recs.h>

#include <innodb/assert/assert.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/machine/data.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/header.h>
#include <innodb/page/page_get_max_trx_id.h>
#include <innodb/page/page_is_leaf.h>
#include <innodb/page/page_zip_compress_write_log.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/record/rec_get_next_offs.h>
#include <innodb/univ/rec_t.h>


#ifndef UNIV_HOTBACKUP
/** Copy the records of a page byte for byte.  Do not copy the page header
 or trailer, except those B-tree header fields that are directly
 related to the storage of records.  Also copy PAGE_MAX_TRX_ID.
 NOTE: The caller must update the lock table and the adaptive hash index. */
void page_zip_copy_recs(
    page_zip_des_t *page_zip,      /*!< out: copy of src_zip
                                   (n_blobs, m_start, m_end,
                                   m_nonempty, data[0..size-1]) */
    page_t *page,                  /*!< out: copy of src */
    const page_zip_des_t *src_zip, /*!< in: compressed page */
    const page_t *src,             /*!< in: page */
    dict_index_t *index,           /*!< in: index of the B-tree */
    mtr_t *mtr)                    /*!< in: mini-transaction */
{
  ut_ad(!index->table->is_temporary());
  ut_ad(mtr_memo_contains_page(mtr, page, MTR_MEMO_PAGE_X_FIX));
  ut_ad(mtr_memo_contains_page(mtr, src, MTR_MEMO_PAGE_X_FIX));
  ut_ad(!dict_index_is_ibuf(index));
#ifdef UNIV_ZIP_DEBUG
  /* The B-tree operations that call this function may set
  FIL_PAGE_PREV or PAGE_LEVEL, causing a temporary min_rec_flag
  mismatch.  A strict page_zip_validate() will be executed later
  during the B-tree operations. */
  ut_a(page_zip_validate_low(src_zip, src, index, TRUE));
#endif /* UNIV_ZIP_DEBUG */
  ut_a(page_zip_get_size(page_zip) == page_zip_get_size(src_zip));
  if (UNIV_UNLIKELY(src_zip->n_blobs)) {
    ut_a(page_is_leaf(src));
    ut_a(index->is_clustered());
  }

  /* The PAGE_MAX_TRX_ID must be set on leaf pages of secondary
  indexes.  It does not matter on other pages. */
  ut_a(index->is_clustered() || !page_is_leaf(src) || page_get_max_trx_id(src));

  UNIV_MEM_ASSERT_W(page, UNIV_PAGE_SIZE);
  UNIV_MEM_ASSERT_W(page_zip->data, page_zip_get_size(page_zip));
  UNIV_MEM_ASSERT_RW(src, UNIV_PAGE_SIZE);
  UNIV_MEM_ASSERT_RW(src_zip->data, page_zip_get_size(page_zip));

  /* Copy those B-tree page header fields that are related to
  the records stored in the page.  Also copy the field
  PAGE_MAX_TRX_ID.  Skip the rest of the page header and
  trailer.  On the compressed page, there is no trailer. */
#if PAGE_MAX_TRX_ID + 8 != PAGE_HEADER_PRIV_END
#error "PAGE_MAX_TRX_ID + 8 != PAGE_HEADER_PRIV_END"
#endif
  memcpy(PAGE_HEADER + page, PAGE_HEADER + src, PAGE_HEADER_PRIV_END);
  memcpy(PAGE_DATA + page, PAGE_DATA + src,
         UNIV_PAGE_SIZE - PAGE_DATA - FIL_PAGE_DATA_END);
  memcpy(PAGE_HEADER + page_zip->data, PAGE_HEADER + src_zip->data,
         PAGE_HEADER_PRIV_END);
  memcpy(PAGE_DATA + page_zip->data, PAGE_DATA + src_zip->data,
         page_zip_get_size(page_zip) - PAGE_DATA);

  /* Copy all fields of src_zip to page_zip, except the pointer
  to the compressed data page. */
  {
    page_zip_t *data = page_zip->data;
    memcpy(page_zip, src_zip, sizeof *page_zip);
    page_zip->data = data;
  }
  ut_ad(page_zip_get_trailer_len(page_zip, index->is_clustered()) +
            page_zip->m_end <
        page_zip_get_size(page_zip));

  if (!page_is_leaf(src) &&
      UNIV_UNLIKELY(mach_read_from_4(src + FIL_PAGE_PREV) == FIL_NULL) &&
      UNIV_LIKELY(mach_read_from_4(page + FIL_PAGE_PREV) != FIL_NULL)) {
    /* Clear the REC_INFO_MIN_REC_FLAG of the first user record. */
    ulint offs = rec_get_next_offs(page + PAGE_NEW_INFIMUM, TRUE);
    if (UNIV_LIKELY(offs != PAGE_NEW_SUPREMUM)) {
      rec_t *rec = page + offs;
      ut_a(rec[-REC_N_NEW_EXTRA_BYTES] & REC_INFO_MIN_REC_FLAG);
      rec[-REC_N_NEW_EXTRA_BYTES] &= ~REC_INFO_MIN_REC_FLAG;
    }
  }

#ifdef UNIV_ZIP_DEBUG
  ut_a(page_zip_validate(page_zip, page, index));
#endif /* UNIV_ZIP_DEBUG */
  page_zip_compress_write_log(page_zip, page, index, mtr);
}
#endif /* !UNIV_HOTBACKUP */
