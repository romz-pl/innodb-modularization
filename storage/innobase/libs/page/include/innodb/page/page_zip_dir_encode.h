#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>
#include <innodb/univ/rec_t.h>

void page_zip_dir_encode(
    const page_t *page, /*!< in: compact page */
    byte *buf,          /*!< in: pointer to dense page directory[-1];
                        out: dense directory on compressed page */
    const rec_t **recs); /*!< in: pointer to an array of 0, or NULL;
                        out: dense page directory sorted by ascending
                        address (and heap_no) */
