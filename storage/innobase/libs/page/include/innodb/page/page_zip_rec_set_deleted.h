#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;

/** Write the "deleted" flag of a record on a compressed page.  The flag must
 already have been written on the uncompressed page. */
void page_zip_rec_set_deleted(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    const byte *rec,          /*!< in: record on the uncompressed page */
    ulint flag);              /*!< in: the deleted flag (nonzero=TRUE) */
