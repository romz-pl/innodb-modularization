#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;

/** Add a slot to the dense page directory. */
void page_zip_dir_add_slot(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    bool is_clustered);       /*!< in: nonzero for clustered index,
                              zero for others */
