#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>
#include <innodb/univ/rec_t.h>

struct page_zip_des_t;
struct dict_index_t;

ulint page_zip_get_n_prev_extern(
    const page_zip_des_t *page_zip, /*!< in: dense page directory on
                                   compressed page */
    const rec_t *rec,               /*!< in: compact physical record
                                    on a B-tree leaf page */
    const dict_index_t *index);      /*!< in: record descriptor */
