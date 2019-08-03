#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;

/** Initialize a compressed page descriptor. */
void page_zip_des_init(page_zip_des_t *page_zip); /*!< in/out: compressed page
                                                  descriptor */
