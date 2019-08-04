#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_ZIP_DEBUG

/** Check that the compressed and decompressed pages match. */
ibool page_zip_validate(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    const page_t *page,             /*!< in: uncompressed page */
    const dict_index_t *index);     /*!< in: index of the page, if known */

#endif
