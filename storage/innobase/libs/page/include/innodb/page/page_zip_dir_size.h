#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;

ulint page_zip_dir_size(
    const page_zip_des_t *page_zip); /*!< in: compressed page */
