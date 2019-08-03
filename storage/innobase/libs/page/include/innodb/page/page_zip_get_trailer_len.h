#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;

ibool page_zip_get_trailer_len(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    bool is_clust);                  /*!< in: TRUE if clustered index */
