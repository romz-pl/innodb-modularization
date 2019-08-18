#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;

/** Insert a record to the dense page directory. */
void page_zip_dir_insert(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    const byte *prev_rec,     /*!< in: record after which to insert */
    const byte *free_rec,     /*!< in: record from which rec was
                             allocated, or NULL */
    byte *rec);               /*!< in: record to insert */
