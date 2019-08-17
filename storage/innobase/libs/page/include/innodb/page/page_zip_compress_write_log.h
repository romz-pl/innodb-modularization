#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;
struct dict_index_t;
struct mtr_t;

void page_zip_compress_write_log(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    const page_t *page,             /*!< in: uncompressed page */
    dict_index_t *index,            /*!< in: index of the B-tree node */
    mtr_t *mtr);                     /*!< in: mini-transaction */
