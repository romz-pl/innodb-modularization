#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;
struct mtr_t;
struct dict_index_t;

/** Write a BLOB pointer of a record on the leaf page of a clustered index.
 The information must already have been updated on the uncompressed page. */
void page_zip_write_blob_ptr(
    page_zip_des_t *page_zip,  /*!< in/out: compressed page */
    const byte *rec,           /*!< in/out: record whose data is being
                               written */
    const dict_index_t *index, /*!< in: index of the page */
    const ulint *offsets,      /*!< in: rec_get_offsets(rec, index) */
    ulint n,                   /*!< in: column index */
    mtr_t *mtr);               /*!< in: mini-transaction handle,
                       or NULL if no logging is needed */
