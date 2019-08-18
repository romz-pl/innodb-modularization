#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;
struct mtr_t;

/** Write the node pointer of a record on a non-leaf compressed page. */
void page_zip_write_node_ptr(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    byte *rec,                /*!< in/out: record */
    ulint size,               /*!< in: data size of rec */
    ulint ptr,                /*!< in: node pointer */
    mtr_t *mtr);              /*!< in: mini-transaction, or NULL */
