#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>


struct page_zip_des_t;

ibool page_zip_set_extra_bytes(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    page_t *page,                   /*!< in/out: uncompressed page */
    ulint info_bits);                /*!< in: REC_INFO_MIN_REC_FLAG or 0 */
