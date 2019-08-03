#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;

lint page_zip_max_ins_size(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    ibool is_clust);                 /*!< in: TRUE if clustered index */
