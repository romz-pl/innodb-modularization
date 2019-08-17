#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;
struct buf_block_t;
struct mtr_t;

/** Sets the max trx id field value. */
void page_set_max_trx_id(
    buf_block_t *block,       /*!< in/out: page */
    page_zip_des_t *page_zip, /*!< in/out: compressed page, or NULL */
    trx_id_t trx_id,          /*!< in: transaction id */
    mtr_t *mtr);              /*!< in/out: mini-transaction, or NULL */
