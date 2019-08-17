#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/disk/page_t.h>

struct page_zip_des_t;

MY_ATTRIBUTE((warn_unused_result)) ibool page_zip_dir_decode(
    const page_zip_des_t *page_zip, /*!< in: dense page directory on
                                   compressed page */
    page_t *page,                   /*!< in: compact page with valid header;
                                    out: trailer and sparse page directory
                                    filled in */
    rec_t **recs,                   /*!< out: dense page directory sorted by
                                    ascending address (and heap_no) */
    ulint n_dense);                  /*!< in: number of user records, and
                                    size of recs[] */
