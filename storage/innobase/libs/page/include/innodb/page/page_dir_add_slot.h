#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;

void page_dir_add_slot(
    page_t *page,             /*!< in/out: the index page */
    page_zip_des_t *page_zip, /*!< in/out: comprssed page, or NULL */
    ulint start) ;
