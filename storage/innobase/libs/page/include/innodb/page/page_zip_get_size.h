#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;

/** Determine the size of a compressed page in bytes.
 @return size in bytes */
ulint page_zip_get_size(
    const page_zip_des_t *page_zip); /*!< in: compressed page */
