#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;
struct dict_index_t;

/** Write an entire record on the compressed page.  The data must already
 have been written to the uncompressed page. */
void page_zip_write_rec(
    page_zip_des_t *page_zip,  /*!< in/out: compressed page */
    const byte *rec,           /*!< in: record being written */
    const dict_index_t *index, /*!< in: the index the record belongs to */
    const ulint *offsets,      /*!< in: rec_get_offsets(rec, index) */
    ulint create);             /*!< in: nonzero=insert, zero=update */
