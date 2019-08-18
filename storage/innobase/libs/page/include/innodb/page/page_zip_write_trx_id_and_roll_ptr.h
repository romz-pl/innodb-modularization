#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;


/** Write the trx_id and roll_ptr of a record on a B-tree leaf node page. */
void page_zip_write_trx_id_and_roll_ptr(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    byte *rec,                /*!< in/out: record */
    const ulint *offsets,     /*!< in: rec_get_offsets(rec, index) */
    ulint trx_id_col,         /*!< in: column number of TRX_ID in rec */
    trx_id_t trx_id,          /*!< in: transaction identifier */
    roll_ptr_t roll_ptr);     /*!< in: roll_ptr */
