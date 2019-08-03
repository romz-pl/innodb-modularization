#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;

byte *page_zip_dir_find_free(
    page_zip_des_t *page_zip, /*!< in: compressed page */
    ulint offset);             /*!< in: offset of user record */
