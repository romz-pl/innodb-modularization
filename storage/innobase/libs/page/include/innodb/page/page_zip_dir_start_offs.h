#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;

ulint page_zip_dir_start_offs(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    ulint n_dense);                  /*!< in: directory size */
