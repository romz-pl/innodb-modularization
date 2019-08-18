#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_ZIP_DEBUG

#include <innodb/disk/page_t.h>

struct page_zip_des_t;

/** Check that the compressed and decompressed pages match.
 @return true if valid, false if not */
ibool page_zip_validate_low(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    const page_t *page,             /*!< in: uncompressed page */
    const dict_index_t *index,      /*!< in: index of the page, if known */
    ibool sloppy);                  /*!< in: FALSE=strict,
                            TRUE=ignore the MIN_REC_FLAG */

#endif                              /* UNIV_ZIP_DEBUG */
