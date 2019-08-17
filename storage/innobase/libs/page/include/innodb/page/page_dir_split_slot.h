#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;

void page_dir_split_slot(
    page_t *page,             /*!< in: index page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page whose
                             uncompressed part will be written, or NULL */
    ulint slot_no);           /*!< in: the directory slot */
