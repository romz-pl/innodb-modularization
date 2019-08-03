#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;

ibool page_zip_available(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    bool is_clust,                  /*!< in: TRUE if clustered index */
    ulint length,                   /*!< in: combined size of the record */
    ulint create);                   /*!< in: nonzero=add the record to
                                    the heap */
