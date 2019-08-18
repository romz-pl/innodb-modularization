#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;
struct dict_index_t;

void page_zip_clear_rec(
    page_zip_des_t *page_zip,  /*!< in/out: compressed page */
    byte *rec,                 /*!< in: record to clear */
    const dict_index_t *index, /*!< in: index of rec */
    const ulint *offsets);      /*!< in: rec_get_offsets(rec, index) */
