#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;

/** Decompress a page.  This function should tolerate errors on the compressed
 page.  Instead of letting assertions fail, it will return FALSE if an
 inconsistency is detected.
 @return true on success, false on failure */
ibool page_zip_decompress_low(
    page_zip_des_t *page_zip, /*!< in: data, ssize;
                             out: m_start, m_end, m_nonempty, n_blobs */
    page_t *page,             /*!< out: uncompressed page, may be trashed */
    ibool all);               /*!< in: TRUE=decompress the whole page;
                              FALSE=verify but do not copy some
                              page header fields that should not change
                              after page creation */
