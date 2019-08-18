#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

ibool page_zip_header_cmp(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    const byte *page);               /*!< in: uncompressed page */

#endif
