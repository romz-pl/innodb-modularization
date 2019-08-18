#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;
struct dict_index_t;
struct mtr_t;

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
    mtr_t *mtr);                   /*!< in: mini-transaction */
